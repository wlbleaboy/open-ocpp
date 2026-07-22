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

#ifndef OPENOCPP_OCPP20_MAINTENANCEMANAGER20_H
#define OPENOCPP_OCPP20_MAINTENANCEMANAGER20_H

#include "FirmwareStatusNotification20.h"
#include "GenericMessageHandler.h"
#include "GetLog20.h"
#include "ITriggerMessageManager20.h"
#include "LogStatusNotification20.h"
#include "PublishFirmware20.h"
#include "PublishFirmwareStatusNotification20.h"
#include "Reset20.h"
#include "SetNetworkProfile20.h"
#include "UnlockConnector20.h"
#include "UnpublishFirmware20.h"
#include "UpdateFirmware20.h"

#include <string>
#include <vector>

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
class GenericMessagesConverter;
class IMessageDispatcher;
} // namespace messages
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 maintenance requests for the charge point */
class MaintenanceManager20
    : public ITriggerMessageManager::ITriggerMessageHandler,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetLogReq,
                                                   ocpp::messages::ocpp20::GetLogConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::PublishFirmwareReq,
                                                   ocpp::messages::ocpp20::PublishFirmwareConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ResetReq, ocpp::messages::ocpp20::ResetConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetNetworkProfileReq,
                                                   ocpp::messages::ocpp20::SetNetworkProfileConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::UnlockConnectorReq,
                                                   ocpp::messages::ocpp20::UnlockConnectorConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::UnpublishFirmwareReq,
                                                   ocpp::messages::ocpp20::UnpublishFirmwareConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::UpdateFirmwareReq,
                                                   ocpp::messages::ocpp20::UpdateFirmwareConf>
{
  public:
    /** @brief Constructor */
    MaintenanceManager20(IChargePointEventsHandler20&                    events_handler,
                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                         ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                         ocpp::messages::GenericMessageSender&           msg_sender,
                         ITriggerMessageManager&                         trigger_manager,
                         ocpp::helpers::WorkerThreadPool&                worker_pool);

    /** @brief Destructor */
    virtual ~MaintenanceManager20();

    /** @brief Notify firmware update status */
    bool notifyFirmwareUpdateStatus(ocpp::types::ocpp20::FirmwareStatusEnumType status, const ocpp::types::Optional<int>& request_id);

    /** @brief Notify log upload status */
    bool notifyLogStatus(ocpp::types::ocpp20::UploadLogStatusEnumType status, const ocpp::types::Optional<int>& request_id);

    /** @brief Notify publish firmware status */
    bool notifyPublishFirmwareStatus(ocpp::types::ocpp20::PublishFirmwareStatusEnumType status,
                                     const std::vector<std::string>&                     locations,
                                     const ocpp::types::Optional<int>&                   request_id);

    // ITriggerMessageManager::ITriggerMessageHandler interface

    bool onTriggerMessage(ocpp::types::ocpp20::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp20::EVSEType>& evse) override;

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::GetLogReq& request,
                       ocpp::messages::ocpp20::GetLogConf&      response,
                       std::string&                             error_code,
                       std::string&                             error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::PublishFirmwareReq& request,
                       ocpp::messages::ocpp20::PublishFirmwareConf&      response,
                       std::string&                                      error_code,
                       std::string&                                      error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::ResetReq& request,
                       ocpp::messages::ocpp20::ResetConf&      response,
                       std::string&                            error_code,
                       std::string&                            error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetNetworkProfileReq& request,
                       ocpp::messages::ocpp20::SetNetworkProfileConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::UnlockConnectorReq& request,
                       ocpp::messages::ocpp20::UnlockConnectorConf&      response,
                       std::string&                                      error_code,
                       std::string&                                      error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::UnpublishFirmwareReq& request,
                       ocpp::messages::ocpp20::UnpublishFirmwareConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::UpdateFirmwareReq& request,
                       ocpp::messages::ocpp20::UpdateFirmwareConf&      response,
                       std::string&                                     error_code,
                       std::string&                                     error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Worker thread pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_MAINTENANCEMANAGER20_H
