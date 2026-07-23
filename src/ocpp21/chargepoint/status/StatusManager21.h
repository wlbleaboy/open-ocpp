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

#ifndef OPENOCPP_OCPP21_STATUSMANAGER21_H
#define OPENOCPP_OCPP21_STATUSMANAGER21_H

#include "BootNotification21.h"
#include "ChangeAvailability21.h"
#include "GenericMessageHandler.h"
#include "Heartbeat21.h"
#include "ITriggerMessageManager21.h"
#include "RegistrationStatusEnumType21.h"
#include "StatusNotification21.h"
#include "Timer.h"

#include <chrono>
#include <map>

namespace ocpp
{
namespace config
{
class IChargePointConfig21;
} // namespace config
namespace helpers
{
class ITimerPool;
class WorkerThreadPool;
} // namespace helpers
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 charge point status messages */
class StatusManager21
    : public ITriggerMessageManager21::ITriggerMessageHandler,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ChangeAvailabilityReq,
                                                   ocpp::messages::ocpp21::ChangeAvailabilityConf>
{
  public:
    StatusManager21(const ocpp::config::IChargePointConfig21&            stack_config,
                    IChargePointEventsHandler21&                         events_handler,
                    ocpp::helpers::ITimerPool&                           timer_pool,
                    ocpp::helpers::WorkerThreadPool&                     worker_pool,
                    const ocpp::messages::GenericMessagesConverter&      messages_converter,
                    ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
                    ocpp::messages::GenericMessageSender&                msg_sender,
                    ITriggerMessageManager21&                            trigger_manager,
                    ocpp::types::ocpp21::BootReasonEnumType              boot_reason);
    virtual ~StatusManager21();

    ocpp::types::ocpp21::RegistrationStatusEnumType getRegistrationStatus() const { return m_registration_status; }
    void forceRegistrationStatus(ocpp::types::ocpp21::RegistrationStatusEnumType status);
    void updateConnectionStatus(bool is_connected);
    void resetHeartBeatTimer();

    bool call(const ocpp::messages::ocpp21::BootNotificationReq& request,
              ocpp::messages::ocpp21::BootNotificationConf&      response,
              std::string&                                       error,
              std::string&                                       message);
    bool call(const ocpp::messages::ocpp21::HeartbeatReq& request,
              ocpp::messages::ocpp21::HeartbeatConf&      response,
              std::string&                                error,
              std::string&                                message);
    bool call(const ocpp::messages::ocpp21::StatusNotificationReq& request,
              ocpp::messages::ocpp21::StatusNotificationConf&      response,
              std::string&                                        error,
              std::string&                                        message);

    bool statusNotification(unsigned int                                 evse_id,
                            unsigned int                                 connector_id,
                            ocpp::types::ocpp21::ConnectorStatusEnumType status);

    bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) override;

    bool handleMessage(const ocpp::messages::ocpp21::ChangeAvailabilityReq& request,
                       ocpp::messages::ocpp21::ChangeAvailabilityConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

  private:
    struct ConnectorStatus
    {
        ocpp::types::ocpp21::ConnectorStatusEnumType status;
        ocpp::types::DateTime timestamp;
    };

    const ocpp::config::IChargePointConfig21& m_stack_config;
    IChargePointEventsHandler21& m_events_handler;
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    const ocpp::types::ocpp21::BootReasonEnumType m_boot_reason;
    ocpp::types::ocpp21::RegistrationStatusEnumType m_registration_status;
    bool m_force_boot_notification;
    bool m_boot_notification_sent;
    ocpp::helpers::Timer m_boot_notification_timer;
    ocpp::helpers::Timer m_heartbeat_timer;
    std::chrono::seconds m_heartbeat_interval;
    std::map<std::pair<unsigned int, unsigned int>, ConnectorStatus> m_connector_statuses;

    void bootNotificationProcess();
    void heartBeatProcess();
    void statusNotificationProcess(unsigned int evse_id, unsigned int connector_id);
    void sendBootNotification(ocpp::types::ocpp21::BootReasonEnumType reason);
    void fillBootNotificationRequest(ocpp::messages::ocpp21::BootNotificationReq& request,
                                     ocpp::types::ocpp21::BootReasonEnumType reason);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_STATUSMANAGER21_H
