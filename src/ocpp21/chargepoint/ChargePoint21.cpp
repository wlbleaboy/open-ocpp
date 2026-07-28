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

#include "ChargePoint21.h"
#include "AuthentManager21.h"
#include "DataTransferManager21.h"
#include "DeviceModelManager21.h"
#include "DeviceModelMessagesManager21.h"
#include "DisplayManager21.h"
#include "StatusManager21.h"
#include "TransactionManager21.h"
#include "MeterValuesManager21.h"
#include "MonitoringManager21.h"
#include "NotifyManager21.h"
#include "RequestFifo21.h"
#include "RequestFifoManager21.h"
#include "ReservationManager21.h"
#include "SecurityManager21.h"
#include "SmartChargingManager21.h"
#include "TariffManager21.h"
#include "TriggerMessageManager21.h"
#include "MaintenanceManager21.h"
#include "Logger.h"
#include "MessageDispatcher.h"
#include "TimerPool.h"
#include "Url.h"
#include "Version.h"
#include "WebsocketFactory.h"
#include "WorkerThreadPool.h"

#include <cstdlib>
#include <ctime>
#include <filesystem>
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

std::unique_ptr<IChargePoint21> IChargePoint21::create(const ocpp::config::IChargePointConfig21& stack_config,
                                                       IChargePointEventsHandler21&              events_handler)
{
    std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool(new ocpp::helpers::TimerPool());
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool =
        std::make_shared<ocpp::helpers::WorkerThreadPool>(2u);
    std::unique_ptr<MessagesConverter21> messages_converter(new MessagesConverter21());
    std::unique_ptr<DeviceModelManager21> device_model_manager(new DeviceModelManager21(stack_config));
    return std::unique_ptr<IChargePoint21>(
        new ChargePoint21(stack_config, std::move(device_model_manager), events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

std::unique_ptr<IChargePoint21> IChargePoint21::create(const ocpp::config::IChargePointConfig21& stack_config,
                                                       IDeviceModel21&                           device_model,
                                                       IChargePointEventsHandler21&              events_handler)
{
    std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool(new ocpp::helpers::TimerPool());
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool =
        std::make_shared<ocpp::helpers::WorkerThreadPool>(2u);
    std::unique_ptr<MessagesConverter21> messages_converter(new MessagesConverter21());
    return std::unique_ptr<IChargePoint21>(
        new ChargePoint21(stack_config, device_model, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

std::unique_ptr<IChargePoint21> IChargePoint21::create(const ocpp::config::IChargePointConfig21&        stack_config,
                                                       IChargePointEventsHandler21&                     events_handler,
                                                       std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool,
                                                       std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool)
{
    std::unique_ptr<MessagesConverter21> messages_converter(new MessagesConverter21());
    return std::unique_ptr<IChargePoint21>(new ChargePoint21(stack_config, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

std::unique_ptr<IChargePoint21> IChargePoint21::create(const ocpp::config::IChargePointConfig21&        stack_config,
                                                       IDeviceModel21&                                  device_model,
                                                       IChargePointEventsHandler21&                     events_handler,
                                                       std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool,
                                                       std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool)
{
    std::unique_ptr<MessagesConverter21> messages_converter(new MessagesConverter21());
    return std::unique_ptr<IChargePoint21>(
        new ChargePoint21(stack_config, device_model, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

ChargePoint21::ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                             IChargePointEventsHandler21&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter)
    : ChargePoint21(stack_config,
                    std::unique_ptr<DeviceModelManager21>(new DeviceModelManager21(stack_config)),
                    events_handler,
                    timer_pool,
                    worker_pool,
                    std::move(messages_converter))
{
}

ChargePoint21::ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                             std::unique_ptr<DeviceModelManager21>&&                         device_model_manager,
                             IChargePointEventsHandler21&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter)
    : ChargePoint21(stack_config,
                    *device_model_manager,
                    events_handler,
                    timer_pool,
                    worker_pool,
                    std::move(messages_converter))
{
    m_device_model_manager = std::move(device_model_manager);
}

ChargePoint21::ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                             IDeviceModel21&                                                device_model,
                             IChargePointEventsHandler21&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter)
    : m_stack_config(stack_config),
      m_events_handler(events_handler),
      m_device_model_manager(),
      m_device_model(&device_model),
      m_timer_pool(timer_pool),
      m_worker_pool(worker_pool),
      m_database(),
      m_messages_converter(std::move(messages_converter)),
      m_messages_validator(),
      m_stop_in_progress(false),
      m_reconnect_scheduled(false),
      m_rpc_connected(false),
      m_ws_client(),
      m_rpc_client(),
      m_msg_dispatcher(),
      m_msg_sender(),
      m_authent_manager(),
      m_data_transfer_manager(),
      m_status_manager(),
      m_transaction_manager(),
      m_meter_values_manager(),
      m_notify_manager(),
      m_smart_charging_manager(),
      m_trigger_manager(),
      m_maintenance_manager(),
      m_display_manager(),
      m_monitoring_manager(),
      m_request_fifo(),
      m_request_fifo_manager(),
      m_reservation_manager(),
      m_security_manager(),
      m_tariff_manager(),
      m_device_model_messages_manager()
{
    if (m_stack_config.databasePath().empty() || m_database.open(m_stack_config.databasePath()))
    {
        if (m_stack_config.logMaxEntriesCount() != 0)
        {
            ocpp::log::Logger::registerDefaultLogger(m_database, m_stack_config.logMaxEntriesCount());
        }
    }
    else
    {
        LOG_ERROR << "Unable to open database";
    }

    std::srand(static_cast<unsigned int>(time(nullptr)));
}

ChargePoint21::~ChargePoint21()
{
    stop();
}

ocpp::helpers::ITimerPool& ChargePoint21::getTimerPool()
{
    return *m_timer_pool;
}

ocpp::helpers::WorkerThreadPool& ChargePoint21::getWorkerPool()
{
    return *m_worker_pool;
}

MeterValuesManager21& ChargePoint21::getMeterValuesManager()
{
    return *m_meter_values_manager;
}

INotifyManager21& ChargePoint21::getNotifyManager()
{
    return *m_notify_manager;
}

ISecurityManager21& ChargePoint21::getSecurityManager()
{
    return *m_security_manager;
}

SmartChargingManager21& ChargePoint21::getSmartChargingManager()
{
    return *m_smart_charging_manager;
}

StatusManager21& ChargePoint21::getStatusManager()
{
    return *m_status_manager;
}

TransactionManager21& ChargePoint21::getTransactionManager()
{
    return *m_transaction_manager;
}

bool ChargePoint21::resetData()
{
    bool ret = false;
    if (!m_rpc_client)
    {
        LOG_INFO << "Reset all data";
        if (!m_stack_config.databasePath().empty())
        {
            if (m_stack_config.logMaxEntriesCount() != 0)
            {
                ocpp::log::Logger::unregisterDefaultLogger();
            }

            m_database.close();

            std::error_code ec;
            const bool removed = std::filesystem::remove(m_stack_config.databasePath(), ec) ||
                                 !std::filesystem::exists(m_stack_config.databasePath());
            if (!ec && removed)
            {
                if (m_database.open(m_stack_config.databasePath()))
                {
                    if (m_stack_config.logMaxEntriesCount() != 0)
                    {
                        ocpp::log::Logger::registerDefaultLogger(m_database, m_stack_config.logMaxEntriesCount());
                    }
                    ret = true;
                }
                else
                {
                    LOG_ERROR << "Unable to open database";
                }
            }
            else
            {
                LOG_ERROR << "Unable to delete database";
            }
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool ChargePoint21::startTransaction(unsigned int                                evse_id,
                                     unsigned int                                connector_id,
                                     const IdTokenType&                          id_token,
                                     TriggerReasonEnumType                       trigger_reason,
                                     std::string&                                transaction_id)
{
    return startTransaction(evse_id, connector_id, id_token, trigger_reason, -1, transaction_id);
}

bool ChargePoint21::startTransaction(unsigned int                                evse_id,
                                     unsigned int                                connector_id,
                                     const IdTokenType&                          id_token,
                                     TriggerReasonEnumType                       trigger_reason,
                                     int                                         remote_start_id,
                                     std::string&                                transaction_id)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->startTransaction(evse_id, connector_id, id_token, trigger_reason, remote_start_id, transaction_id);
    }
    return ret;
}

bool ChargePoint21::startTransaction(unsigned int evse_id, unsigned int connector_id, const std::string& id_token, std::string& transaction_id)
{
    IdTokenType token;
    token.idToken.assign(id_token);
    token.type.assign("ISO14443");

    return startTransaction(evse_id, connector_id, token, TriggerReasonEnumType::Authorized, transaction_id);
}

bool ChargePoint21::updateTransaction(const std::string&                                      transaction_id,
                                      TriggerReasonEnumType                                   trigger_reason,
                                      const std::vector<MeterValueType>&                       meter_values)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->updateTransaction(transaction_id, trigger_reason, meter_values);
    }
    return ret;
}

bool ChargePoint21::hasActiveTransaction(const std::string& transaction_id)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->hasActiveTransaction(transaction_id);
    }
    return ret;
}

bool ChargePoint21::stopTransaction(const std::string&                                      transaction_id,
                                    ReasonEnumType                                         reason,
                                    TriggerReasonEnumType                                  trigger_reason,
                                    const IdTokenType*                                     id_token,
                                    const std::vector<MeterValueType>&                      meter_values)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->stopTransaction(transaction_id, reason, trigger_reason, id_token, meter_values);
    }
    return ret;
}

bool ChargePoint21::stopTransaction(const std::string& transaction_id, const std::string& id_token, ReasonEnumType reason)
{
    Optional<IdTokenType> token;
    if (!id_token.empty())
    {
        token.value().idToken.assign(id_token);
        token.value().type.assign("ISO14443");
    }

    return stopTransaction(transaction_id, reason, TriggerReasonEnumType::StopAuthorized, token.isSet() ? &token.value() : nullptr, {});
}

void ChargePoint21::startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->startPeriodicMeterValues(evse_id, interval);
    }
}

void ChargePoint21::stopPeriodicMeterValues(unsigned int evse_id)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->stopPeriodicMeterValues(evse_id);
    }
}

void ChargePoint21::startTransactionSampledMeterValues(const std::string& transaction_id,
                                                       unsigned int       evse_id,
                                                       std::chrono::seconds interval)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->startTransactionSampledMeterValues(transaction_id, evse_id, interval);
    }
}

void ChargePoint21::stopTransactionSampledMeterValues(const std::string& transaction_id)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->stopTransactionSampledMeterValues(transaction_id);
    }
}

void ChargePoint21::getTxStopMeterValues(const std::string& transaction_id, std::vector<MeterValueType>& meter_values)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->getTxStopMeterValues(transaction_id, meter_values);
    }
    else
    {
        meter_values.clear();
    }
}

bool ChargePoint21::getSetpoint(unsigned int                                               evse_id,
                                Optional<SmartChargingManager21::SmartChargingSetpoint>&   charging_station_setpoint,
                                Optional<SmartChargingManager21::SmartChargingSetpoint>&   evse_setpoint,
                                ChargingRateUnitEnumType                                   unit)
{
    bool ret = false;
    if (m_smart_charging_manager)
    {
        ret = m_smart_charging_manager->getSetpoint(evse_id, charging_station_setpoint, evse_setpoint, unit);
    }
    return ret;
}

bool ChargePoint21::statusNotification(unsigned int evse_id, unsigned int connector_id, ConnectorStatusEnumType status)
{
    bool ret = false;
    if (m_status_manager)
    {
        ret = m_status_manager->statusNotification(evse_id, connector_id, status);
    }
    return ret;
}

bool ChargePoint21::start()
{
    bool ret = false;
    if (!m_rpc_client)
    {
        LOG_INFO << "Starting OCPP stack v" << OPEN_OCPP_VERSION << " - Central System : " << m_stack_config.connexionUrl()
                 << " - Charge Point identifier : " << m_stack_config.chargePointIdentifier();

        ret = m_messages_validator.load(m_stack_config.jsonSchemasPath());
        if (ret)
        {
            m_ws_client  = std::unique_ptr<ocpp::websockets::IWebsocketClient>(ocpp::websockets::WebsocketFactory::newClient());
            m_rpc_client = std::make_unique<ocpp::rpc::RpcClient>(*m_ws_client, "ocpp2.1");
            m_rpc_client->registerListener(*this);
            m_rpc_client->registerClientListener(*this);
            m_rpc_client->registerSpy(*this);
            m_msg_dispatcher = std::make_unique<ocpp::messages::MessageDispatcher>(m_messages_validator);
            m_msg_sender     = std::make_unique<ocpp::messages::GenericMessageSender>(
                *m_rpc_client, *m_messages_converter, m_messages_validator, m_stack_config.callRequestTimeout());

            m_request_fifo = std::make_unique<RequestFifo21>(m_database);
            m_device_model_messages_manager =
                std::make_unique<DeviceModelMessagesManager21>(*m_device_model, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_authent_manager = std::make_unique<AuthentManager21>(
                m_database, m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_data_transfer_manager = std::make_unique<DataTransferManager21>(
                m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_trigger_manager = std::make_unique<TriggerMessageManager21>(
                m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_notify_manager = std::make_unique<NotifyManager21>(*m_msg_sender);
            m_monitoring_manager = std::make_unique<MonitoringManager21>(
                m_events_handler, *m_messages_converter, *m_msg_dispatcher);
            m_reservation_manager = std::make_unique<ReservationManager21>(
                m_database, m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender, *m_timer_pool, *m_worker_pool);
            m_security_manager = std::make_unique<SecurityManager21>(
                m_stack_config, m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender, *m_trigger_manager, *m_worker_pool);
            m_status_manager = std::make_unique<StatusManager21>(
                m_stack_config,
                m_events_handler,
                *m_timer_pool,
                *m_worker_pool,
                *m_messages_converter,
                *m_msg_dispatcher,
                *m_msg_sender,
                *m_trigger_manager,
                ocpp::types::ocpp21::BootReasonEnumType::PowerUp);
            m_transaction_manager = std::make_unique<TransactionManager21>(
                m_events_handler,
                *m_messages_converter,
                *m_msg_dispatcher,
                *m_msg_sender,
                *m_request_fifo,
                *m_authent_manager,
                m_stack_config.txUpdatedInterval(),
                *m_trigger_manager,
                *m_worker_pool,
                m_reservation_manager.get());

            m_meter_values_manager = std::make_unique<MeterValuesManager21>(
                m_database, m_events_handler, *m_timer_pool, *m_worker_pool, *m_msg_sender, *m_trigger_manager, *m_transaction_manager);
            m_request_fifo_manager = std::make_unique<RequestFifoManager21>(
                m_events_handler, *m_timer_pool, *m_worker_pool, *m_msg_sender, *m_request_fifo);
            m_transaction_manager->setMeterValuesManager(*m_meter_values_manager);
            m_smart_charging_manager = std::make_unique<SmartChargingManager21>(
                m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_maintenance_manager = std::make_unique<MaintenanceManager21>(
                m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender, *m_trigger_manager, *m_worker_pool);
            m_display_manager = std::make_unique<DisplayManager21>(m_events_handler, *m_messages_converter, *m_msg_dispatcher);
            m_tariff_manager = std::make_unique<TariffManager21>(m_events_handler, *m_messages_converter, *m_msg_dispatcher);

            m_stop_in_progress = false;
            ret                = doConnect();
        }
        else
        {
            LOG_ERROR << "Unable to load all the messages validators";
        }
    }
    else
    {
        LOG_ERROR << "Stack already started";
    }
    return ret;
}

bool ChargePoint21::stop()
{
    bool ret = false;
    if (m_rpc_client && !m_stop_in_progress)
    {
        LOG_INFO << "Stopping OCPP stack";
        m_stop_in_progress = true;
        m_rpc_connected = false;
        ret = m_rpc_client->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        m_request_fifo_manager.reset();
        m_security_manager.reset();
        m_reservation_manager.reset();
        m_monitoring_manager.reset();
        m_tariff_manager.reset();
        m_display_manager.reset();
        m_maintenance_manager.reset();
        m_trigger_manager.reset();
        m_smart_charging_manager.reset();
        m_notify_manager.reset();
        m_meter_values_manager.reset();
        m_transaction_manager.reset();
        m_status_manager.reset();
        m_data_transfer_manager.reset();
        m_authent_manager.reset();
        m_device_model_messages_manager.reset();
        m_request_fifo.reset();
        m_msg_sender.reset();
        m_msg_dispatcher.reset();
        m_rpc_client.reset();
        m_ws_client.reset();
        m_database.close();
    }
    else if (m_rpc_client)
    {
        LOG_ERROR << "Stack stop already in progress";
    }
    return ret;
}

bool ChargePoint21::reconnect()
{
    bool ret = false;
    if (m_rpc_client)
    {
        LOG_INFO << "Reconnect triggered";
        scheduleReconnect();
        ret = true;
    }
    else
    {
        LOG_ERROR << "Stack stopped";
    }
    return ret;
}

bool ChargePoint21::authorize(const IdTokenType& id_token, IdTokenInfoType& token_info, std::string& error, std::string& message)
{
    bool ret = false;
    if (m_authent_manager)
    {
        ret = m_authent_manager->authorize(id_token, token_info, error, message);
    }
    else
    {
        error = "NotSupported";
        message = "Authent manager is not available";
    }
    return ret;
}

bool ChargePoint21::call(const VatNumberValidationReq& request,
                         VatNumberValidationConf&      response,
                         std::string&     error,
                         std::string&     message)
{
    return sendCall(VATNUMBERVALIDATION_ACTION, request, response, error, message);
}

void ChargePoint21::rpcClientConnected()
{
    if (m_rpc_connected.exchange(true))
    {
        LOG_DEBUG << "Duplicate connection notification from Central System ignored";
        return;
    }

    LOG_INFO << "Connected to Central System";
    m_events_handler.connectionStateChanged(true);
    if (m_status_manager)
    {
        m_status_manager->updateConnectionStatus(true);
    }
    if (m_request_fifo_manager)
    {
        m_request_fifo_manager->updateConnectionStatus(true);
    }
}

void ChargePoint21::rpcClientFailed()
{
    m_rpc_connected = false;
    LOG_ERROR << "Connection failed with Central System";
    m_events_handler.connectionFailed();
}

void ChargePoint21::rpcDisconnected()
{
    if (!m_stop_in_progress)
    {
        if (!m_rpc_connected.exchange(false))
        {
            LOG_DEBUG << "Duplicate disconnection notification from Central System ignored";
            return;
        }

        LOG_ERROR << "Connection lost with Central System";
        m_events_handler.connectionStateChanged(false);
        if (m_status_manager)
        {
            m_status_manager->updateConnectionStatus(false);
        }
    }
}

void ChargePoint21::rpcError()
{
    LOG_ERROR << "Connection error with Central System";
}

bool ChargePoint21::rpcCallReceived(const std::string&      action,
                                    const rapidjson::Value& payload,
                                    rapidjson::Document&    response,
                                    std::string&            error_code,
                                    std::string&            error_message)
{
    bool ret = false;
    if (!m_stop_in_progress && m_msg_dispatcher)
    {
        ret = m_msg_dispatcher->dispatchMessage(action, payload, response, error_code, error_message);
    }
    return ret;
}

void ChargePoint21::rcpMessageReceived(const std::string& msg)
{
    LOG_COM << "RX : " << msg;
}

void ChargePoint21::rcpMessageSent(const std::string& msg)
{
    LOG_COM << "TX : " << msg;
}


void ChargePoint21::scheduleReconnect()
{
    if (!m_reconnect_scheduled)
    {
        m_reconnect_scheduled = true;
        m_worker_pool->run<void>(
            [this]
            {
                std::this_thread::sleep_for(std::chrono::seconds(1u));
                doConnect();
            });
    }
}

bool ChargePoint21::doConnect()
{
    bool ret = false;
    if (m_rpc_client)
    {
        if (m_rpc_client->isConnected())
        {
            m_rpc_connected = false;
            m_rpc_client->stop();
        }

        const unsigned int security_profile = m_stack_config.securityProfile();
        std::string connection_url = m_stack_config.connexionUrl();
        if (!connection_url.empty() && (connection_url[connection_url.size() - 1] != '/'))
        {
            connection_url += "/";
        }
        connection_url += ocpp::websockets::Url::encode(m_stack_config.chargePointIdentifier());

        ocpp::websockets::IWebsocketClient::Credentials credentials;
        std::string basic_auth_password = m_stack_config.basicAuthPassword();
        if (!basic_auth_password.empty() && (security_profile <= 2))
        {
            credentials.user     = m_stack_config.chargePointIdentifier();
            credentials.password = basic_auth_password;
        }
        if (security_profile != 1)
        {
            credentials.tls12_cipher_list = m_stack_config.tlsv12CipherList();
            credentials.tls13_cipher_list = m_stack_config.tlsv13CipherList();
            credentials.server_certificate_ca = m_stack_config.tlsServerCertificateCa();
            if ((security_profile == 0) || (security_profile == 3))
            {
                credentials.client_certificate                        = m_stack_config.tlsClientCertificate();
                credentials.client_certificate_private_key            = m_stack_config.tlsClientCertificatePrivateKey();
                credentials.client_certificate_private_key_passphrase = m_stack_config.tlsClientCertificatePrivateKeyPassphrase();
            }
            credentials.allow_selfsigned_certificates = m_stack_config.tlsAllowSelfSignedCertificates();
            credentials.allow_expired_certificates    = m_stack_config.tlsAllowExpiredCertificates();
            credentials.accept_untrusted_certificates = m_stack_config.tlsAcceptNonTrustedCertificates();
            credentials.skip_server_name_check        = m_stack_config.tlsSkipServerNameCheck();
            credentials.encoded_pem_certificates      = false;
        }

        m_reconnect_scheduled = false;
        ret = m_rpc_client->start(connection_url,
                                  credentials,
                                  m_stack_config.connectionTimeout(),
                                  m_stack_config.retryInterval(),
                                  m_stack_config.webSocketPingInterval());
    }
    return ret;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
