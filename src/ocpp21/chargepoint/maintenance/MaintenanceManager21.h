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

#ifndef OPENOCPP_OCPP21_MAINTENANCEMANAGER21_H
#define OPENOCPP_OCPP21_MAINTENANCEMANAGER21_H

#include "FirmwareStatusNotification21.h"
#include "GetLog21.h"
#include "LogStatusNotification21.h"
#include "PublishFirmware21.h"
#include "PublishFirmwareStatusNotification21.h"
#include "Reset21.h"
#include "SetNetworkProfile21.h"
#include "UnlockConnector21.h"
#include "UnpublishFirmware21.h"
#include "UpdateFirmware21.h"
#include "GenericMessageHandler.h"
#include "ITriggerMessageManager21.h"

#include <thread>
#include <vector>

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers
namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 maintenance requests for the charge point */
class MaintenanceManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetLogReq, ocpp::messages::ocpp21::GetLogConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::PublishFirmwareReq, ocpp::messages::ocpp21::PublishFirmwareConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ResetReq, ocpp::messages::ocpp21::ResetConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetNetworkProfileReq, ocpp::messages::ocpp21::SetNetworkProfileConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::UnlockConnectorReq, ocpp::messages::ocpp21::UnlockConnectorConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::UnpublishFirmwareReq, ocpp::messages::ocpp21::UnpublishFirmwareConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::UpdateFirmwareReq, ocpp::messages::ocpp21::UpdateFirmwareConf>,
      public ITriggerMessageManager21::ITriggerMessageHandler
{
  public:
    MaintenanceManager21(IChargePointEventsHandler21&                         events_handler,
          const ocpp::messages::GenericMessagesConverter&      messages_converter,
          ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
          ocpp::messages::GenericMessageSender&                msg_sender,
          ITriggerMessageManager21&                            trigger_manager,
          ocpp::helpers::WorkerThreadPool&                     worker_pool);
    virtual ~MaintenanceManager21();

    bool call(const ocpp::messages::ocpp21::FirmwareStatusNotificationReq& request,
              ocpp::messages::ocpp21::FirmwareStatusNotificationConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::LogStatusNotificationReq& request,
              ocpp::messages::ocpp21::LogStatusNotificationConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::PublishFirmwareStatusNotificationReq& request,
              ocpp::messages::ocpp21::PublishFirmwareStatusNotificationConf&      response,
              std::string&                              error,
              std::string&                              message);

    bool notifyFirmwareUpdateStatus(ocpp::types::ocpp21::FirmwareStatusEnumType status,
                                    const ocpp::types::Optional<int>&            request_id);
    bool notifyLogStatus(ocpp::types::ocpp21::UploadLogStatusEnumType status,
                         const ocpp::types::Optional<int>&            request_id);
    bool notifyPublishFirmwareStatus(ocpp::types::ocpp21::PublishFirmwareStatusEnumType status,
                                     const std::vector<std::string>&                      locations,
                                     const ocpp::types::Optional<int>&                    request_id);

    bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) override;

    bool handleMessage(const ocpp::messages::ocpp21::GetLogReq& request,
                       ocpp::messages::ocpp21::GetLogConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::PublishFirmwareReq& request,
                       ocpp::messages::ocpp21::PublishFirmwareConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::ResetReq& request,
                       ocpp::messages::ocpp21::ResetConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetNetworkProfileReq& request,
                       ocpp::messages::ocpp21::SetNetworkProfileConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::UnlockConnectorReq& request,
                       ocpp::messages::ocpp21::UnlockConnectorConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::UnpublishFirmwareReq& request,
                       ocpp::messages::ocpp21::UnpublishFirmwareConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::UpdateFirmwareReq& request,
                       ocpp::messages::ocpp21::UpdateFirmwareConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;

  private:
    IChargePointEventsHandler21& m_events_handler;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    std::thread* m_firmware_thread;
    ocpp::types::ocpp21::FirmwareStatusEnumType m_firmware_status;
    ocpp::types::Optional<int> m_firmware_request_id;

    void processUpdateFirmware(std::string                                  location,
                               std::string                                  local_firmware_file,
                               ocpp::types::Optional<int>                   retries,
                               ocpp::types::Optional<int>                   retry_interval,
                               ocpp::types::DateTime                        retrieve_date,
                               ocpp::types::Optional<ocpp::types::DateTime> install_date,
                               int                                          request_id);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_MAINTENANCEMANAGER21_H
