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

#include "SecurityManager21.h"

#include "CertificateRequest.h"
#include "GenericMessageSender.h"
#include "IChargePointConfig21.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "PrivateKey.h"
#include "WorkerThreadPool.h"

#include <chrono>
#include <thread>

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types;
using namespace ocpp::types::ocpp21;
using namespace ocpp::x509;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

SecurityManager21::SecurityManager21(const ocpp::config::IChargePointConfig21&       stack_config,
                                     IChargePointEventsHandler21&                    events_handler,
                                     const ocpp::messages::GenericMessagesConverter& messages_converter,
                                     ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                     ocpp::messages::GenericMessageSender&           msg_sender,
                                     ITriggerMessageManager21&                       trigger_manager,
                                     ocpp::helpers::WorkerThreadPool&                worker_pool)
    : GenericMessageHandler<CertificateSignedReq, CertificateSignedConf>(CERTIFICATESIGNED_ACTION, messages_converter),
      GenericMessageHandler<CustomerInformationReq, CustomerInformationConf>(CUSTOMERINFORMATION_ACTION, messages_converter),
      GenericMessageHandler<DeleteCertificateReq, DeleteCertificateConf>(DELETECERTIFICATE_ACTION, messages_converter),
      GenericMessageHandler<Get15118EVCertificateReq, Get15118EVCertificateConf>(GET15118EVCERTIFICATE_ACTION, messages_converter),
      GenericMessageHandler<GetCertificateChainStatusReq, GetCertificateChainStatusConf>(GETCERTIFICATECHAINSTATUS_ACTION,
                                                                                         messages_converter),
      GenericMessageHandler<GetCertificateStatusReq, GetCertificateStatusConf>(GETCERTIFICATESTATUS_ACTION, messages_converter),
      GenericMessageHandler<GetInstalledCertificateIdsReq, GetInstalledCertificateIdsConf>(GETINSTALLEDCERTIFICATEIDS_ACTION,
                                                                                           messages_converter),
      GenericMessageHandler<InstallCertificateReq, InstallCertificateConf>(INSTALLCERTIFICATE_ACTION, messages_converter),
      m_stack_config(stack_config),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_worker_pool(worker_pool)
{
    msg_dispatcher.registerHandler(CERTIFICATESIGNED_ACTION,
                                   *dynamic_cast<GenericMessageHandler<CertificateSignedReq, CertificateSignedConf>*>(this));
    msg_dispatcher.registerHandler(CUSTOMERINFORMATION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<CustomerInformationReq, CustomerInformationConf>*>(this));
    msg_dispatcher.registerHandler(DELETECERTIFICATE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<DeleteCertificateReq, DeleteCertificateConf>*>(this));
    msg_dispatcher.registerHandler(
        GET15118EVCERTIFICATE_ACTION,
        *dynamic_cast<GenericMessageHandler<Get15118EVCertificateReq, Get15118EVCertificateConf>*>(this));
    msg_dispatcher.registerHandler(
        GETCERTIFICATECHAINSTATUS_ACTION,
        *dynamic_cast<GenericMessageHandler<GetCertificateChainStatusReq, GetCertificateChainStatusConf>*>(this));
    msg_dispatcher.registerHandler(GETCERTIFICATESTATUS_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetCertificateStatusReq, GetCertificateStatusConf>*>(this));
    msg_dispatcher.registerHandler(
        GETINSTALLEDCERTIFICATEIDS_ACTION,
        *dynamic_cast<GenericMessageHandler<GetInstalledCertificateIdsReq, GetInstalledCertificateIdsConf>*>(this));
    msg_dispatcher.registerHandler(INSTALLCERTIFICATE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<InstallCertificateReq, InstallCertificateConf>*>(this));

    trigger_manager.registerHandler(MessageTriggerEnumType::SignChargingStationCertificate, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::SignV2GCertificate, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::SignV2G20Certificate, *this);
    trigger_manager.registerHandler(MessageTriggerEnumType::SignCombinedCertificate, *this);
}

SecurityManager21::~SecurityManager21() { }

bool SecurityManager21::securityEventNotification(const SecurityEventNotificationReq& request,
                                                  SecurityEventNotificationConf&      response,
                                                  std::string&                        error,
                                                  std::string&                        message)
{
    return call(SECURITYEVENTNOTIFICATION_ACTION, request, response, error, message);
}

bool SecurityManager21::signCertificate(const SignCertificateReq& request,
                                        SignCertificateConf&      response,
                                        std::string&              error,
                                        std::string&              message)
{
    return call(SIGNCERTIFICATE_ACTION, request, response, error, message);
}

bool SecurityManager21::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
{
    (void)evse;

    bool ret = true;
    switch (message)
    {
        case MessageTriggerEnumType::SignChargingStationCertificate:
        case MessageTriggerEnumType::SignV2GCertificate:
        case MessageTriggerEnumType::SignV2G20Certificate:
        case MessageTriggerEnumType::SignCombinedCertificate:
            m_worker_pool.run<void>(
                [this, message]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    triggerSignCertificate(message);
                });
            break;

        default:
            ret = false;
            break;
    }
    return ret;
}

bool SecurityManager21::handleMessage(const CertificateSignedReq& request,
                                      CertificateSignedConf&      response,
                                      std::string&                error_code,
                                      std::string&                error_message)
{
    LOG_INFO << "CertificateSigned request received";
    return m_events_handler.onCertificateSigned(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const CustomerInformationReq& request,
                                      CustomerInformationConf&      response,
                                      std::string&                  error_code,
                                      std::string&                  error_message)
{
    LOG_INFO << "CustomerInformation request received";
    return m_events_handler.onCustomerInformation(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const DeleteCertificateReq& request,
                                      DeleteCertificateConf&      response,
                                      std::string&                error_code,
                                      std::string&                error_message)
{
    LOG_INFO << "DeleteCertificate request received";
    return m_events_handler.onDeleteCertificate(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const Get15118EVCertificateReq& request,
                                      Get15118EVCertificateConf&      response,
                                      std::string&                    error_code,
                                      std::string&                    error_message)
{
    LOG_INFO << "Get15118EVCertificate request received";
    return m_events_handler.onGet15118EVCertificate(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const GetCertificateChainStatusReq& request,
                                      GetCertificateChainStatusConf&      response,
                                      std::string&                        error_code,
                                      std::string&                        error_message)
{
    LOG_INFO << "GetCertificateChainStatus request received";
    return m_events_handler.onGetCertificateChainStatus(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const GetCertificateStatusReq& request,
                                      GetCertificateStatusConf&      response,
                                      std::string&                   error_code,
                                      std::string&                   error_message)
{
    LOG_INFO << "GetCertificateStatus request received";
    return m_events_handler.onGetCertificateStatus(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const GetInstalledCertificateIdsReq& request,
                                      GetInstalledCertificateIdsConf&      response,
                                      std::string&                        error_code,
                                      std::string&                        error_message)
{
    LOG_INFO << "GetInstalledCertificateIds request received";
    return m_events_handler.onGetInstalledCertificateIds(request, response, error_code, error_message);
}

bool SecurityManager21::handleMessage(const InstallCertificateReq& request,
                                      InstallCertificateConf&      response,
                                      std::string&                 error_code,
                                      std::string&                 error_message)
{
    LOG_INFO << "InstallCertificate request received";
    return m_events_handler.onInstallCertificate(request, response, error_code, error_message);
}

template <typename RequestType, typename ResponseType>
bool SecurityManager21::call(const std::string& action,
                             const RequestType& request,
                             ResponseType&      response,
                             std::string&       error,
                             std::string&       message)
{
    const CallResult result = m_msg_sender.call(action, request, response, error, message);
    if (result != CallResult::Ok)
    {
        LOG_ERROR << action << " => " << (result == CallResult::Failed ? "Timeout" : "Error");
    }
    return (result == CallResult::Ok);
}

bool SecurityManager21::triggerSignCertificate(MessageTriggerEnumType message)
{
    bool ret = false;

    PrivateKey private_key(PrivateKey::Type::EC,
                           static_cast<unsigned int>(PrivateKey::Curve::PRIME256_V1),
                           m_stack_config.tlsClientCertificatePrivateKeyPassphrase());
    if (private_key.isValid())
    {
        if (!m_stack_config.tlsClientCertificatePrivateKey().empty() &&
            !private_key.privateToFile(m_stack_config.tlsClientCertificatePrivateKey()))
        {
            LOG_WARNING << "Unable to save generated private key for SignCertificate trigger";
        }

        CertificateRequest::Subject subject;
        subject.organization      = m_stack_config.chargePointVendor();
        subject.organization_unit = m_stack_config.chargePointModel();
        subject.common_name       = m_stack_config.chargePointSerialNumber().empty() ? m_stack_config.chargePointIdentifier()
                                                                                     : m_stack_config.chargePointSerialNumber();

        CertificateRequest certificate_request(subject, private_key);
        if (certificate_request.isValid())
        {
            SignCertificateReq request;
            request.csr.assign(certificate_request.pem());
            if (message == MessageTriggerEnumType::SignChargingStationCertificate)
            {
                request.certificateType = CertificateSigningUseEnumType::ChargingStationCertificate;
            }
            else if (message == MessageTriggerEnumType::SignV2GCertificate)
            {
                request.certificateType = CertificateSigningUseEnumType::V2GCertificate;
            }
            else if (message == MessageTriggerEnumType::SignV2G20Certificate)
            {
                request.certificateType = CertificateSigningUseEnumType::V2G20Certificate;
            }

            SignCertificateConf response;
            std::string         error;
            std::string         error_msg;
            ret = signCertificate(request, response, error, error_msg);
        }
        else
        {
            LOG_ERROR << "Unable to generate CSR for SignCertificate trigger";
        }
    }
    else
    {
        LOG_ERROR << "Unable to generate private key for SignCertificate trigger";
    }

    return ret;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
