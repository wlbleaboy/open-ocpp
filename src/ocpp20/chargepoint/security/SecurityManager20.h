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

#ifndef OPENOCPP_OCPP20_SECURITYMANAGER20_H
#define OPENOCPP_OCPP20_SECURITYMANAGER20_H

#include "CertificateSigned20.h"
#include "DeleteCertificate20.h"
#include "GenericMessageHandler.h"
#include "Get15118EVCertificate20.h"
#include "GetCertificateStatus20.h"
#include "GetInstalledCertificateIds20.h"
#include "InstallCertificate20.h"
#include "ISecurityManager20.h"
#include "ITriggerMessageManager20.h"

#include <string>

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
class GenericMessagesConverter;
class IMessageDispatcher;
} // namespace messages
namespace config
{
class IChargePointConfig20;
} // namespace config
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 security and certificate messages for the charge point */
class SecurityManager20
    : public ISecurityManager20,
      public ITriggerMessageManager::ITriggerMessageHandler,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::CertificateSignedReq,
                                                   ocpp::messages::ocpp20::CertificateSignedConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::DeleteCertificateReq,
                                                   ocpp::messages::ocpp20::DeleteCertificateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::Get15118EVCertificateReq,
                                                   ocpp::messages::ocpp20::Get15118EVCertificateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetCertificateStatusReq,
                                                   ocpp::messages::ocpp20::GetCertificateStatusConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetInstalledCertificateIdsReq,
                                                   ocpp::messages::ocpp20::GetInstalledCertificateIdsConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::InstallCertificateReq,
                                                   ocpp::messages::ocpp20::InstallCertificateConf>
{
  public:
    /** @brief Constructor */
    SecurityManager20(const ocpp::config::IChargePointConfig20&       stack_config,
                      IChargePointEventsHandler20&                    events_handler,
                      const ocpp::messages::GenericMessagesConverter& messages_converter,
                      ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                      ocpp::messages::GenericMessageSender&           msg_sender,
                      ITriggerMessageManager&                         trigger_manager,
                      ocpp::helpers::WorkerThreadPool&                worker_pool);

    /** @brief Destructor */
    virtual ~SecurityManager20();

    // ISecurityManager20 interface

    bool securityEventNotification(const ocpp::messages::ocpp20::SecurityEventNotificationReq& request,
                                   ocpp::messages::ocpp20::SecurityEventNotificationConf&      response,
                                   std::string&                                                error,
                                   std::string&                                                message) override;

    bool signCertificate(const ocpp::messages::ocpp20::SignCertificateReq& request,
                         ocpp::messages::ocpp20::SignCertificateConf&      response,
                         std::string&                                      error,
                         std::string&                                      message) override;

    // ITriggerMessageManager::ITriggerMessageHandler interface

    bool onTriggerMessage(ocpp::types::ocpp20::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp20::EVSEType>& evse) override;

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::CertificateSignedReq& request,
                       ocpp::messages::ocpp20::CertificateSignedConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::DeleteCertificateReq& request,
                       ocpp::messages::ocpp20::DeleteCertificateConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::Get15118EVCertificateReq& request,
                       ocpp::messages::ocpp20::Get15118EVCertificateConf&      response,
                       std::string&                                            error_code,
                       std::string&                                            error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetCertificateStatusReq& request,
                       ocpp::messages::ocpp20::GetCertificateStatusConf&      response,
                       std::string&                                           error_code,
                       std::string&                                           error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetInstalledCertificateIdsReq& request,
                       ocpp::messages::ocpp20::GetInstalledCertificateIdsConf&      response,
                       std::string&                                                 error_code,
                       std::string&                                                 error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::InstallCertificateReq& request,
                       ocpp::messages::ocpp20::InstallCertificateConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

  private:
    /** @brief Stack configuration */
    const ocpp::config::IChargePointConfig20& m_stack_config;
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Worker thread pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;

    /** @brief Execute a security call */
    template <typename RequestType, typename ResponseType>
    bool call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message);

    /** @brief Send a SignCertificate request generated for a trigger message */
    bool triggerSignCertificate(ocpp::types::ocpp20::MessageTriggerEnumType message);
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_SECURITYMANAGER20_H
