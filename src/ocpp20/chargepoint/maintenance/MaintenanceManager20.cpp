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

#include "MaintenanceManager20.h"

#include "GenericMessageSender.h"
#include "IChargePointEventsHandler20.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "WorkerThreadPool.h"

#include <chrono>
#include <thread>

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;
using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
MaintenanceManager20::MaintenanceManager20(IChargePointEventsHandler20&                    events_handler,
                                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                                           ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                           ocpp::messages::GenericMessageSender&           msg_sender,
                                           ITriggerMessageManager&                         trigger_manager,
                                           ocpp::helpers::WorkerThreadPool&                worker_pool)
    : GenericMessageHandler<GetLogReq, GetLogConf>(GETLOG_ACTION, messages_converter),
      GenericMessageHandler<PublishFirmwareReq, PublishFirmwareConf>(PUBLISHFIRMWARE_ACTION, messages_converter),
      GenericMessageHandler<ResetReq, ResetConf>(RESET_ACTION, messages_converter),
      GenericMessageHandler<SetNetworkProfileReq, SetNetworkProfileConf>(SETNETWORKPROFILE_ACTION, messages_converter),
      GenericMessageHandler<UnlockConnectorReq, UnlockConnectorConf>(UNLOCKCONNECTOR_ACTION, messages_converter),
      GenericMessageHandler<UnpublishFirmwareReq, UnpublishFirmwareConf>(UNPUBLISHFIRMWARE_ACTION, messages_converter),
      GenericMessageHandler<UpdateFirmwareReq, UpdateFirmwareConf>(UPDATEFIRMWARE_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_worker_pool(worker_pool)
{
    msg_dispatcher.registerHandler(GETLOG_ACTION, *dynamic_cast<GenericMessageHandler<GetLogReq, GetLogConf>*>(this));
    msg_dispatcher.registerHandler(PUBLISHFIRMWARE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<PublishFirmwareReq, PublishFirmwareConf>*>(this));
    msg_dispatcher.registerHandler(RESET_ACTION, *dynamic_cast<GenericMessageHandler<ResetReq, ResetConf>*>(this));
    msg_dispatcher.registerHandler(SETNETWORKPROFILE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetNetworkProfileReq, SetNetworkProfileConf>*>(this));
    msg_dispatcher.registerHandler(UNLOCKCONNECTOR_ACTION,
                                   *dynamic_cast<GenericMessageHandler<UnlockConnectorReq, UnlockConnectorConf>*>(this));
    msg_dispatcher.registerHandler(UNPUBLISHFIRMWARE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<UnpublishFirmwareReq, UnpublishFirmwareConf>*>(this));
    msg_dispatcher.registerHandler(UPDATEFIRMWARE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<UpdateFirmwareReq, UpdateFirmwareConf>*>(this));

    trigger_manager.registerHandler(MessageTriggerEnumType::LogStatusNotification, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::FirmwareStatusNotification, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::PublishFirmwareStatusNotification, *this);
}

/** @brief Destructor */
MaintenanceManager20::~MaintenanceManager20() { }

/** @brief Notify firmware update status */
bool MaintenanceManager20::notifyFirmwareUpdateStatus(FirmwareStatusEnumType status, const Optional<int>& request_id)
{
    FirmwareStatusNotificationReq request;
    request.status = status;
    request.requestId = request_id;

    FirmwareStatusNotificationConf response;
    CallResult result = m_msg_sender.call(FIRMWARESTATUSNOTIFICATION_ACTION, request, response);
    return (result == CallResult::Ok);
}

/** @brief Notify log upload status */
bool MaintenanceManager20::notifyLogStatus(UploadLogStatusEnumType status, const Optional<int>& request_id)
{
    LogStatusNotificationReq request;
    request.status = status;
    request.requestId = request_id;

    LogStatusNotificationConf response;
    CallResult result = m_msg_sender.call(LOGSTATUSNOTIFICATION_ACTION, request, response);
    return (result == CallResult::Ok);
}

/** @brief Notify publish firmware status */
bool MaintenanceManager20::notifyPublishFirmwareStatus(PublishFirmwareStatusEnumType status,
                                                       const std::vector<std::string>& locations,
                                                       const Optional<int>&             request_id)
{
    PublishFirmwareStatusNotificationReq request;
    request.status    = status;
    request.location  = locations;
    request.requestId = request_id;

    PublishFirmwareStatusNotificationConf response;
    CallResult result = m_msg_sender.call(PUBLISHFIRMWARESTATUSNOTIFICATION_ACTION, request, response);
    return (result == CallResult::Ok);
}

/** @copydoc bool ITriggerMessageManager::ITriggerMessageHandler::onTriggerMessage(...) */
bool MaintenanceManager20::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
{
    (void)evse;

    bool ret = true;

    switch (message)
    {
        case MessageTriggerEnumType::LogStatusNotification:
        {
            m_worker_pool.run<void>(
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    Optional<int> request_id;
                    notifyLogStatus(UploadLogStatusEnumType::Idle, request_id);
                });
        }
        break;

        case MessageTriggerEnumType::FirmwareStatusNotification:
        {
            m_worker_pool.run<void>(
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    Optional<int> request_id;
                    notifyFirmwareUpdateStatus(FirmwareStatusEnumType::Idle, request_id);
                });
        }
        break;

        case MessageTriggerEnumType::PublishFirmwareStatusNotification:
        {
            m_worker_pool.run<void>(
                [this]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    Optional<int> request_id;
                    notifyPublishFirmwareStatus(PublishFirmwareStatusEnumType::Idle, {}, request_id);
                });
        }
        break;

        default:
            ret = false;
            break;
    }

    return ret;
}

/** @brief Handle GetLog */
bool MaintenanceManager20::handleMessage(const GetLogReq& request,
                                         GetLogConf&      response,
                                         std::string&     error_code,
                                         std::string&     error_message)
{
    LOG_INFO << "GetLog request received : requestId = " << request.requestId << " - type = "
             << LogEnumTypeHelper.toString(request.logType);
    return m_events_handler.onGetLog(request, response, error_code, error_message);
}

/** @brief Handle PublishFirmware */
bool MaintenanceManager20::handleMessage(const PublishFirmwareReq& request,
                                         PublishFirmwareConf&      response,
                                         std::string&              error_code,
                                         std::string&              error_message)
{
    LOG_INFO << "PublishFirmware request received : requestId = " << request.requestId << " - location = " << request.location.str();
    return m_events_handler.onPublishFirmware(request, response, error_code, error_message);
}

/** @brief Handle Reset */
bool MaintenanceManager20::handleMessage(const ResetReq& request,
                                         ResetConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    LOG_INFO << "Reset request received : type = " << ResetEnumTypeHelper.toString(request.type)
             << " - evseId = " << (request.evseId.isSet() ? std::to_string(request.evseId.value()) : "not set");
    return m_events_handler.onReset(request, response, error_code, error_message);
}

/** @brief Handle SetNetworkProfile */
bool MaintenanceManager20::handleMessage(const SetNetworkProfileReq& request,
                                         SetNetworkProfileConf&      response,
                                         std::string&                error_code,
                                         std::string&                error_message)
{
    LOG_INFO << "SetNetworkProfile request received : slot = " << request.configurationSlot;
    return m_events_handler.onSetNetworkProfile(request, response, error_code, error_message);
}

/** @brief Handle UnlockConnector */
bool MaintenanceManager20::handleMessage(const UnlockConnectorReq& request,
                                         UnlockConnectorConf&      response,
                                         std::string&              error_code,
                                         std::string&              error_message)
{
    LOG_INFO << "UnlockConnector request received : evseId = " << request.evseId << " - connectorId = " << request.connectorId;
    return m_events_handler.onUnlockConnector(request, response, error_code, error_message);
}

/** @brief Handle UnpublishFirmware */
bool MaintenanceManager20::handleMessage(const UnpublishFirmwareReq& request,
                                         UnpublishFirmwareConf&      response,
                                         std::string&                error_code,
                                         std::string&                error_message)
{
    LOG_INFO << "UnpublishFirmware request received : checksum = " << request.checksum.str();
    return m_events_handler.onUnpublishFirmware(request, response, error_code, error_message);
}

/** @brief Handle UpdateFirmware */
bool MaintenanceManager20::handleMessage(const UpdateFirmwareReq& request,
                                         UpdateFirmwareConf&      response,
                                         std::string&             error_code,
                                         std::string&             error_message)
{
    LOG_INFO << "UpdateFirmware request received : requestId = " << request.requestId
             << " - location = " << request.firmware.location.str();
    return m_events_handler.onUpdateFirmware(request, response, error_code, error_message);
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
