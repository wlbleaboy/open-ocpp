/*
Copyright (c) 2020 Cedric Jimenez
This file is part of OpenOCPP.

OpenOCPP is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

OpenOCPP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenOCPP. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StatusManager21.h"
#include "DateTime.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "IChargePointConfig21.h"
#include "IMessageDispatcher.h"
#include "ITimerPool.h"
#include "Logger.h"
#include "WorkerThreadPool.h"

#include <functional>
#include <thread>

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types;
using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

StatusManager21::StatusManager21(const ocpp::config::IChargePointConfig21&       stack_config,
                                 IChargePointEventsHandler21&                    events_handler,
                                 ocpp::helpers::ITimerPool&                      timer_pool,
                                 ocpp::helpers::WorkerThreadPool&                worker_pool,
                                 const ocpp::messages::GenericMessagesConverter& messages_converter,
                                 ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                 ocpp::messages::GenericMessageSender&           msg_sender,
                                 ITriggerMessageManager21&                       trigger_manager,
                                 BootReasonEnumType                              boot_reason)
    : GenericMessageHandler<ChangeAvailabilityReq, ChangeAvailabilityConf>(CHANGEAVAILABILITY_ACTION, messages_converter),
      m_stack_config(stack_config),
      m_events_handler(events_handler),
      m_worker_pool(worker_pool),
      m_msg_sender(msg_sender),
      m_boot_reason(boot_reason),
      m_registration_status(RegistrationStatusEnumType::Rejected),
      m_force_boot_notification(false),
      m_boot_notification_sent(false),
      m_boot_notification_timer(timer_pool, "OCPP2.1 Boot notification"),
      m_heartbeat_timer(timer_pool, "OCPP2.1 Heartbeat"),
      m_heartbeat_interval(std::chrono::hours(1)),
      m_connector_statuses()
{
    m_boot_notification_timer.setCallback([this] { m_worker_pool.run<void>(std::bind(&StatusManager21::bootNotificationProcess, this)); });
    m_heartbeat_timer.setCallback([this] { m_worker_pool.run<void>(std::bind(&StatusManager21::heartBeatProcess, this)); });

    trigger_manager.registerHandler(MessageTriggerEnumType::BootNotification, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::Heartbeat, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::StatusNotification, *this);

    msg_dispatcher.registerHandler(CHANGEAVAILABILITY_ACTION,
                                   *dynamic_cast<GenericMessageHandler<ChangeAvailabilityReq, ChangeAvailabilityConf>*>(this));
}

StatusManager21::~StatusManager21()
{
    m_boot_notification_timer.stop();
    m_heartbeat_timer.stop();
}

void StatusManager21::forceRegistrationStatus(RegistrationStatusEnumType status)
{
    if (status != m_registration_status)
    {
        m_force_boot_notification = true;
        m_boot_notification_sent  = false;
    }
    m_registration_status = status;
}

void StatusManager21::updateConnectionStatus(bool is_connected)
{
    if (is_connected)
    {
        if (m_registration_status == RegistrationStatusEnumType::Accepted)
        {
            m_force_boot_notification = true;
        }

        if (m_force_boot_notification || (m_registration_status != RegistrationStatusEnumType::Accepted))
        {
            m_boot_notification_timer.start(std::chrono::milliseconds(1u), true);
        }
        else
        {
            for (const auto& connector : m_connector_statuses)
            {
                statusNotificationProcess(connector.first.first, connector.first.second);
            }
            m_heartbeat_timer.start(m_heartbeat_interval);
        }
    }
    else
    {
        m_boot_notification_sent = false;
        m_boot_notification_timer.stop();
        m_heartbeat_timer.stop();
    }
}

void StatusManager21::resetHeartBeatTimer()
{
    if (m_heartbeat_timer.isStarted())
    {
        m_heartbeat_timer.restart(m_heartbeat_interval);
    }
}

bool StatusManager21::call(const BootNotificationReq& request, BootNotificationConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(BOOTNOTIFICATION_ACTION, request, response, error, message) == CallResult::Ok);
}

bool StatusManager21::call(const HeartbeatReq& request, HeartbeatConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(HEARTBEAT_ACTION, request, response, error, message) == CallResult::Ok);
}

bool StatusManager21::call(const StatusNotificationReq& request, StatusNotificationConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(STATUSNOTIFICATION_ACTION, request, response, error, message) == CallResult::Ok);
}

bool StatusManager21::statusNotification(unsigned int evse_id,
                                         unsigned int connector_id,
                                         ConnectorStatusEnumType status)
{
    ConnectorStatus connector_status;
    connector_status.status    = status;
    connector_status.timestamp = DateTime::now();
    m_connector_statuses[{evse_id, connector_id}] = connector_status;

    return statusNotificationProcess(evse_id, connector_id), true;
}

bool StatusManager21::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
{
    bool ret = true;

    switch (message)
    {
        case MessageTriggerEnumType::BootNotification:
        {
            m_worker_pool.run<void>(
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    sendBootNotification(BootReasonEnumType::Triggered);
                });
        }
        break;

        case MessageTriggerEnumType::Heartbeat:
        {
            m_worker_pool.run<void>(
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    heartBeatProcess();
                });
        }
        break;

        case MessageTriggerEnumType::StatusNotification:
        {
            if (evse.isSet())
            {
                if (evse.value().connectorId.isSet())
                {
                    const unsigned int evse_id      = static_cast<unsigned int>(evse.value().id);
                    const unsigned int connector_id = static_cast<unsigned int>(evse.value().connectorId.value());
                    m_worker_pool.run<void>(
                        [this, evse_id, connector_id]
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                            statusNotificationProcess(evse_id, connector_id);
                        });
                }
                else
                {
                    bool found = false;
                    const unsigned int evse_id = static_cast<unsigned int>(evse.value().id);
                    for (const auto& connector : m_connector_statuses)
                    {
                        if (connector.first.first == evse_id)
                        {
                            found = true;
                            const unsigned int connector_id = connector.first.second;
                            m_worker_pool.run<void>(
                                [this, evse_id, connector_id]
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                                    statusNotificationProcess(evse_id, connector_id);
                                });
                        }
                    }
                    if (!found)
                    {
                        ret = false;
                    }
                }
            }
            else
            {
                for (const auto& connector : m_connector_statuses)
                {
                    const unsigned int evse_id      = connector.first.first;
                    const unsigned int connector_id = connector.first.second;
                    m_worker_pool.run<void>(
                        [this, evse_id, connector_id]
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                            statusNotificationProcess(evse_id, connector_id);
                        });
                }
            }
        }
        break;

        default:
            ret = false;
            break;
    }

    return ret;
}

bool StatusManager21::handleMessage(const ChangeAvailabilityReq& request,
                                    ChangeAvailabilityConf&      response,
                                    std::string&                 error_code,
                                    std::string&                 error_message)
{
    return m_events_handler.onChangeAvailability(request, response, error_code, error_message);
}

void StatusManager21::bootNotificationProcess()
{
    if (!m_boot_notification_sent)
    {
        sendBootNotification(m_boot_reason);
    }
    else
    {
        for (const auto& connector : m_connector_statuses)
        {
            statusNotificationProcess(connector.first.first, connector.first.second);
        }
        m_heartbeat_timer.start(m_heartbeat_interval);
    }
}

void StatusManager21::heartBeatProcess()
{
    HeartbeatReq  heartbeat_req;
    HeartbeatConf heartbeat_conf;
    CallResult    result = m_msg_sender.call(HEARTBEAT_ACTION, heartbeat_req, heartbeat_conf);
    if (result == CallResult::Ok)
    {
        LOG_INFO << "Heartbeat : " << heartbeat_conf.currentTime.str();
        m_events_handler.datetimeReceived(heartbeat_conf.currentTime);
        if (m_heartbeat_timer.isSingleShot())
        {
            m_heartbeat_timer.restart(m_heartbeat_interval);
        }
    }
}

void StatusManager21::statusNotificationProcess(unsigned int evse_id, unsigned int connector_id)
{
    auto connector = m_connector_statuses.find({evse_id, connector_id});
    if (connector == m_connector_statuses.end())
    {
        ConnectorStatus connector_status;
        connector_status.status    = ConnectorStatusEnumType::Available;
        connector_status.timestamp = DateTime::now();
        connector = m_connector_statuses.emplace(std::make_pair(evse_id, connector_id), connector_status).first;
    }

    StatusNotificationReq status_req;
    status_req.evseId          = static_cast<int>(evse_id);
    status_req.connectorId     = static_cast<int>(connector_id);
    status_req.connectorStatus = connector->second.status;
    status_req.timestamp       = connector->second.timestamp;

    StatusNotificationConf status_conf;
    m_msg_sender.call(STATUSNOTIFICATION_ACTION, status_req, status_conf);
}

void StatusManager21::sendBootNotification(BootReasonEnumType reason)
{
    BootNotificationReq boot_req;
    fillBootNotificationRequest(boot_req, reason);

    BootNotificationConf boot_conf;
    CallResult           result = m_msg_sender.call(BOOTNOTIFICATION_ACTION, boot_req, boot_conf);
    if (result == CallResult::Ok)
    {
        m_registration_status = boot_conf.status;
        LOG_INFO << "Registration status : " << RegistrationStatusEnumTypeHelper.toString(m_registration_status);

        m_force_boot_notification = false;
        if (m_registration_status == RegistrationStatusEnumType::Accepted)
        {
            m_boot_notification_sent = true;
            m_boot_notification_timer.stop();
            m_heartbeat_interval = std::chrono::seconds(boot_conf.interval);
            m_heartbeat_timer.start(m_heartbeat_interval);

            for (const auto& connector : m_connector_statuses)
            {
                statusNotificationProcess(connector.first.first, connector.first.second);
            }
        }
        else
        {
            m_boot_notification_sent = false;
            m_boot_notification_timer.start(std::chrono::seconds(boot_conf.interval), true);
        }

        m_events_handler.bootNotification(m_registration_status, boot_conf.currentTime);
    }
    else
    {
        m_boot_notification_timer.start(m_stack_config.retryInterval(), true);
    }
}

void StatusManager21::fillBootNotificationRequest(BootNotificationReq& request, BootReasonEnumType reason)
{
    request.reason = reason;
    request.chargingStation.model.assign(m_stack_config.chargePointModel());
    request.chargingStation.vendorName.assign(m_stack_config.chargePointVendor());
    if (!m_stack_config.chargePointSerialNumber().empty())
    {
        request.chargingStation.serialNumber.value().assign(m_stack_config.chargePointSerialNumber());
    }
    if (!m_stack_config.firmwareVersion().empty())
    {
        request.chargingStation.firmwareVersion.value().assign(m_stack_config.firmwareVersion());
    }
    if (!m_stack_config.imsi().empty())
    {
        request.chargingStation.modem.value().imsi.value().assign(m_stack_config.imsi());
    }
    if (!m_stack_config.iccid().empty())
    {
        request.chargingStation.modem.value().iccid.value().assign(m_stack_config.iccid());
    }
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
