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

#ifndef OPENOCPP_OCPP21_SECURITYMANAGER21_H
#define OPENOCPP_OCPP21_SECURITYMANAGER21_H

#include "CertificateSigned21.h"
#include "DeleteCertificate21.h"
#include "GenericMessageHandler.h"
#include "Get15118EVCertificate21.h"
#include "GetCertificateChainStatus21.h"
#include "GetCertificateStatus21.h"
#include "GetInstalledCertificateIds21.h"
#include "InstallCertificate21.h"
#include "ISecurityManager21.h"
#include "ITriggerMessageManager21.h"
#include "CustomerInformation21.h"

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
class IChargePointConfig21;
} // namespace config
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 security and certificate messages for the charge point */
class SecurityManager21
    : public ISecurityManager21,
      public ITriggerMessageManager21::ITriggerMessageHandler,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::CertificateSignedReq,
                                                   ocpp::messages::ocpp21::CertificateSignedConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::CustomerInformationReq,
                                                   ocpp::messages::ocpp21::CustomerInformationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::DeleteCertificateReq,
                                                   ocpp::messages::ocpp21::DeleteCertificateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::Get15118EVCertificateReq,
                                                   ocpp::messages::ocpp21::Get15118EVCertificateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetCertificateChainStatusReq,
                                                   ocpp::messages::ocpp21::GetCertificateChainStatusConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetCertificateStatusReq,
                                                   ocpp::messages::ocpp21::GetCertificateStatusConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetInstalledCertificateIdsReq,
                                                   ocpp::messages::ocpp21::GetInstalledCertificateIdsConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::InstallCertificateReq,
                                                   ocpp::messages::ocpp21::InstallCertificateConf>
{
  public:
    SecurityManager21(const ocpp::config::IChargePointConfig21&       stack_config,
                      IChargePointEventsHandler21&                    events_handler,
                      const ocpp::messages::GenericMessagesConverter& messages_converter,
                      ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                      ocpp::messages::GenericMessageSender&           msg_sender,
                      ITriggerMessageManager21&                       trigger_manager,
                      ocpp::helpers::WorkerThreadPool&                worker_pool);
    virtual ~SecurityManager21();

    bool securityEventNotification(const ocpp::messages::ocpp21::SecurityEventNotificationReq& request,
                                   ocpp::messages::ocpp21::SecurityEventNotificationConf&      response,
                                   std::string&                                                error,
                                   std::string&                                                message) override;

    bool signCertificate(const ocpp::messages::ocpp21::SignCertificateReq& request,
                         ocpp::messages::ocpp21::SignCertificateConf&      response,
                         std::string&                                      error,
                         std::string&                                      message) override;

    bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) override;

    bool handleMessage(const ocpp::messages::ocpp21::CertificateSignedReq& request,
                       ocpp::messages::ocpp21::CertificateSignedConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::CustomerInformationReq& request,
                       ocpp::messages::ocpp21::CustomerInformationConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::DeleteCertificateReq& request,
                       ocpp::messages::ocpp21::DeleteCertificateConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::Get15118EVCertificateReq& request,
                       ocpp::messages::ocpp21::Get15118EVCertificateConf&      response,
                       std::string&                                            error_code,
                       std::string&                                            error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetCertificateChainStatusReq& request,
                       ocpp::messages::ocpp21::GetCertificateChainStatusConf&      response,
                       std::string&                                                error_code,
                       std::string&                                                error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetCertificateStatusReq& request,
                       ocpp::messages::ocpp21::GetCertificateStatusConf&      response,
                       std::string&                                           error_code,
                       std::string&                                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetInstalledCertificateIdsReq& request,
                       ocpp::messages::ocpp21::GetInstalledCertificateIdsConf&      response,
                       std::string&                                                 error_code,
                       std::string&                                                 error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::InstallCertificateReq& request,
                       ocpp::messages::ocpp21::InstallCertificateConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

  private:
    const ocpp::config::IChargePointConfig21& m_stack_config;
    IChargePointEventsHandler21&              m_events_handler;
    ocpp::messages::GenericMessageSender&     m_msg_sender;
    ocpp::helpers::WorkerThreadPool&          m_worker_pool;

    template <typename RequestType, typename ResponseType>
    bool call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message);

    bool triggerSignCertificate(ocpp::types::ocpp21::MessageTriggerEnumType message);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_SECURITYMANAGER21_H
