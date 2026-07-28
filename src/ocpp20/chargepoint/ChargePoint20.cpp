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

#include "ChargePoint20.h"
#include "AuthentManager20.h"
#include "DataTransferManager20.h"
#include "DeviceModelManager20.h"
#include "DeviceModelMessagesManager20.h"
#include "InternalConfigKeys.h"
#include "Logger.h"
#include "MaintenanceManager20.h"
#include "MeterValuesManager20.h"
#include "MessageDispatcher.h"
#include "MonitoringManager20.h"
#include "NotifyManager20.h"
#include "PrivateKey.h"
#include "RequestFifo20.h"
#include "RequestFifoManager20.h"
#include "ReservationManager20.h"
#include "SecurityManager20.h"
#include "SmartChargingManager20.h"
#include "StatusManager20.h"
#include "TimerPool.h"
#include "TransactionManager20.h"
#include "Url.h"
#include "Version.h"
#include "WebsocketFactory.h"
#include "WorkerThreadPool.h"
#include "TriggerMessageManager20.h"

#include <filesystem>
#include <iostream>
#include <random>

using namespace ocpp::types;
using namespace ocpp::types::ocpp20;
using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Instanciate a charge point */
std::unique_ptr<IChargePoint20> IChargePoint20::create(const ocpp::config::IChargePointConfig20& stack_config,
                                                       IChargePointEventsHandler20&              events_handler)
{
    std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool(new ocpp::helpers::TimerPool());
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool =
        std::make_shared<ocpp::helpers::WorkerThreadPool>(2u); // 1 asynchronous timer operations + 1 for asynchronous jobs/responses
    std::unique_ptr<MessagesConverter20> messages_converter(new MessagesConverter20());
    std::unique_ptr<DeviceModelManager>   device_model_manager(new DeviceModelManager(stack_config));
    return std::unique_ptr<IChargePoint20>(
        new ChargePoint20(stack_config,
                          std::move(device_model_manager),
                          events_handler,
                          timer_pool,
                          worker_pool,
                          std::move(messages_converter)));
}

/** @brief Instanciate a charge point with a device model */
std::unique_ptr<IChargePoint20> IChargePoint20::create(const ocpp::config::IChargePointConfig20& stack_config,
                                                       IDeviceModel&                             device_model,
                                                       IChargePointEventsHandler20&              events_handler)
{
    std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool(new ocpp::helpers::TimerPool());
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool =
        std::make_shared<ocpp::helpers::WorkerThreadPool>(2u);
    std::unique_ptr<MessagesConverter20> messages_converter(new MessagesConverter20());
    return std::unique_ptr<IChargePoint20>(
        new ChargePoint20(stack_config, device_model, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

/** @brief Instanciate a charge point with the provided timer and worker pools */
std::unique_ptr<IChargePoint20> IChargePoint20::create(const ocpp::config::IChargePointConfig20&        stack_config,
                                                       IChargePointEventsHandler20&                     events_handler,
                                                       std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool,
                                                       std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool)
{
    std::unique_ptr<MessagesConverter20> messages_converter(new MessagesConverter20());
    return std::unique_ptr<IChargePoint20>(
        new ChargePoint20(stack_config, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

/** @brief Instanciate a charge point with the provided timer and worker pools and a device model */
std::unique_ptr<IChargePoint20> IChargePoint20::create(const ocpp::config::IChargePointConfig20&        stack_config,
                                                       IDeviceModel&                                    device_model,
                                                       IChargePointEventsHandler20&                     events_handler,
                                                       std::shared_ptr<ocpp::helpers::ITimerPool>       timer_pool,
                                                       std::shared_ptr<ocpp::helpers::WorkerThreadPool> worker_pool)
{
    std::unique_ptr<MessagesConverter20> messages_converter(new MessagesConverter20());
    return std::unique_ptr<IChargePoint20>(
        new ChargePoint20(stack_config, device_model, events_handler, timer_pool, worker_pool, std::move(messages_converter)));
}

/** @brief Constructor */
ChargePoint20::ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                             IChargePointEventsHandler20&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter)
    : ChargePoint20(stack_config,
                    std::unique_ptr<DeviceModelManager>(new DeviceModelManager(stack_config)),
                    events_handler,
                    timer_pool,
                    worker_pool,
                    std::move(messages_converter))
{
}

/** @brief Constructor */
ChargePoint20::ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                             std::unique_ptr<DeviceModelManager>&&                          device_model_manager,
                             IChargePointEventsHandler20&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter)
    : ChargePoint20(stack_config,
                    *device_model_manager,
                    events_handler,
                    timer_pool,
                    worker_pool,
                    std::move(messages_converter))
{
    m_device_model_manager = std::move(device_model_manager);
}

/** @brief Constructor */
ChargePoint20::ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                             IDeviceModel&                                                  device_model,
                             IChargePointEventsHandler20&                                   events_handler,
                             std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                             std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                             std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter)
    : GenericMessageHandler<ClearDisplayMessageReq, ClearDisplayMessageConf>(CLEARDISPLAYMESSAGE_ACTION, *messages_converter),
      GenericMessageHandler<CustomerInformationReq, CustomerInformationConf>(CUSTOMERINFORMATION_ACTION, *messages_converter),
      GenericMessageHandler<GetDisplayMessagesReq, GetDisplayMessagesConf>(GETDISPLAYMESSAGES_ACTION, *messages_converter),
      GenericMessageHandler<SetDisplayMessageReq, SetDisplayMessageConf>(SETDISPLAYMESSAGE_ACTION, *messages_converter),

      m_stack_config(stack_config),
      m_events_handler(events_handler),
      m_device_model_manager(),
      m_device_model(&device_model),
      m_timer_pool(timer_pool),
      m_worker_pool(worker_pool),
      m_database(),
      m_internal_config(m_database),
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
      m_connectors(*m_device_model, m_database, *m_timer_pool.get()),
      m_trigger_manager(),
      m_status_manager(),
      m_requests_fifo(),
      m_requests_fifo_manager(),
      m_device_model_messages_manager(),
      m_monitoring_manager(),
      m_reservation_manager(),
      m_transaction_manager(),
      m_meter_values_manager(),
      m_notify_manager(),
      m_security_manager(),
      m_data_transfer_manager(),
      m_uptime_timer(*m_timer_pool.get(), "Uptime timer"),
      m_uptime(0),
      m_disconnected_time(0),
      m_total_uptime(0),
      m_total_disconnected_time(0)
{
    // Open database
    if (m_stack_config.databasePath().empty() || m_database.open(m_stack_config.databasePath()))
    {
        // Register logger
        if (m_stack_config.logMaxEntriesCount() != 0)
        {
            ocpp::log::Logger::registerDefaultLogger(m_database, m_stack_config.logMaxEntriesCount());
        }

        // Initialize the database
        if (!m_stack_config.databasePath().empty())
        {
            initDatabase();
        }
    }
    else
    {
        LOG_ERROR << "Unable to open database";
    }

    // Uptime timer
    m_uptime_timer.setCallback(std::bind(&ChargePoint20::processUptime, this));

    // Random numbers
    std::srand(static_cast<unsigned int>(time(nullptr)));
}

/** @brief Destructor */
ChargePoint20::~ChargePoint20()
{
    stop();
}

/** @copydoc INotifyManager20& IChargePoint20::getNotifyManager() */
INotifyManager20& ChargePoint20::getNotifyManager()
{
    return *m_notify_manager;
}

/** @copydoc ISecurityManager20& IChargePoint20::getSecurityManager() */
ISecurityManager20& ChargePoint20::getSecurityManager()
{
    return *m_security_manager;
}

/** @copydoc ISmartChargingManager20& IChargePoint20::getSmartChargingManager() */
ISmartChargingManager20& ChargePoint20::getSmartChargingManager()
{
    return *m_smart_charging_manager;
}

/** @copydoc ITransactionManager20& IChargePoint20::getTransactionManager() */
ITransactionManager20& ChargePoint20::getTransactionManager()
{
    return *m_transaction_manager;
}

/** @copydoc bool IChargePoint20::resetData() */
bool ChargePoint20::resetData()
{
    bool ret = false;

    // Check if it is already started
    if (!m_rpc_client)
    {
        LOG_INFO << "Reset all data";
        if (!m_stack_config.databasePath().empty())
        {
            // Unregister logger
            if (m_stack_config.logMaxEntriesCount() != 0)
            {
                ocpp::log::Logger::unregisterDefaultLogger();
            }

            // Close database to invalid existing connexions
            m_database.close();

            // Delete database
            if (std::filesystem::remove(m_stack_config.databasePath()))
            {
                // Open database
                if (m_database.open(m_stack_config.databasePath()))
                {
                    // Register logger
                    if (m_stack_config.logMaxEntriesCount() != 0)
                    {
                        ocpp::log::Logger::registerDefaultLogger(m_database, m_stack_config.logMaxEntriesCount());
                    }

                    // Re-initialize with default values
                    m_total_uptime            = 0;
                    m_total_disconnected_time = 0;
                    initDatabase();
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

/** @copydoc bool IChargePoint20::start() */
bool ChargePoint20::start()
{
    bool ret = false;

    // Check if it is already started
    if (!m_rpc_client)
    {

        LOG_INFO << "Starting OCPP stack v" << OPEN_OCPP_VERSION << " - Central System : " << m_stack_config.connexionUrl()
                 << " - Charge Point identifier : " << m_stack_config.chargePointIdentifier();

        // Load validator
        ret = m_messages_validator.load(m_stack_config.jsonSchemasPath());
        if (ret)
        {
            // Start uptime counter
            if (!m_stack_config.databasePath().empty())
            {
                m_uptime            = 0;
                m_disconnected_time = 0;
                m_internal_config.setKey(START_DATE_KEY, DateTime::now().str());
                m_uptime_timer.start(std::chrono::seconds(1u));
            }

            // Allocate resources
            m_ws_client  = std::unique_ptr<ocpp::websockets::IWebsocketClient>(ocpp::websockets::WebsocketFactory::newClient());
            m_rpc_client = std::make_unique<ocpp::rpc::RpcClient>(*m_ws_client, "ocpp2.0.1");
            m_rpc_client->registerListener(*this);
            m_rpc_client->registerClientListener(*this);
            m_rpc_client->registerSpy(*this);
            m_msg_dispatcher = std::make_unique<ocpp::messages::MessageDispatcher>(m_messages_validator);
            m_msg_sender     = std::make_unique<ocpp::messages::GenericMessageSender>(
                *m_rpc_client, *m_messages_converter, m_messages_validator, m_stack_config.callRequestTimeout());
            m_authent_manager = std::make_unique<AuthentManager20>(
                m_database, m_events_handler, m_internal_config, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_connectors.initDatabaseTable();
            m_requests_fifo         = std::make_unique<RequestFifo20>(m_database);
            m_requests_fifo_manager =
                std::make_unique<RequestFifoManager20>(m_events_handler, *m_timer_pool, *m_worker_pool, *m_msg_sender, *m_requests_fifo);
            m_device_model_messages_manager =
                std::make_unique<DeviceModelMessagesManager20>(m_events_handler, *m_messages_converter, *m_msg_dispatcher);
            m_monitoring_manager =
                std::make_unique<MonitoringManager20>(m_events_handler, *m_messages_converter, *m_msg_dispatcher);
            m_trigger_manager = std::make_unique<TriggerMessageManager>(m_connectors, *m_messages_converter, *m_msg_dispatcher);
            m_status_manager  = std::make_unique<StatusManager>(m_stack_config,
                                                               *m_device_model,
                                                               m_events_handler,
                                                               m_internal_config,
                                                               *m_timer_pool,
                                                               *m_worker_pool,
                                                               m_connectors,
                                                               *m_msg_dispatcher,
                                                               *m_messages_converter,
                                                               *m_msg_sender,
                                                               *m_trigger_manager,
                                                               BootReasonEnumType::PowerUp);
            m_maintenance_manager =
                std::make_unique<MaintenanceManager20>(m_events_handler,
                                                       *m_messages_converter,
                                                       *m_msg_dispatcher,
                                                       *m_msg_sender,
                                                       *m_trigger_manager,
                                                       *m_worker_pool);
            m_reservation_manager   = std::make_unique<ReservationManager20>(
                m_database, m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender, *m_timer_pool, *m_worker_pool);
            m_smart_charging_manager =
                std::make_unique<SmartChargingManager20>(
                    m_database, m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);
            m_transaction_manager =
                std::make_unique<TransactionManager20>(
                    m_database,
                    m_events_handler,
                    *m_messages_converter,
                    *m_msg_dispatcher,
                    *m_msg_sender,
                    *m_requests_fifo,
                    *m_authent_manager,
                    m_stack_config.txUpdatedInterval(),
                    *m_trigger_manager,
                    *m_worker_pool,
                    m_reservation_manager.get(),
                    m_smart_charging_manager.get());
            m_meter_values_manager =
                std::make_unique<MeterValuesManager20>(m_database,
                                                       m_events_handler,
                                                       *m_timer_pool,
                                                       *m_worker_pool,
                                                       *m_msg_sender,
                                                       *m_requests_fifo,
                                                       *m_trigger_manager,
                                                       *m_transaction_manager);
            m_transaction_manager->setMeterValuesManager(*m_meter_values_manager);
            m_notify_manager = std::make_unique<NotifyManager20>(*m_msg_sender);
            m_security_manager =
                std::make_unique<SecurityManager20>(m_stack_config,
                                                    m_events_handler,
                                                    *m_messages_converter,
                                                    *m_msg_dispatcher,
                                                    *m_msg_sender,
                                                    *m_trigger_manager,
                                                    *m_worker_pool);
            m_data_transfer_manager =
                std::make_unique<DataTransferManager20>(m_events_handler, *m_messages_converter, *m_msg_dispatcher, *m_msg_sender);

            // Register to Central System messages
            m_msg_dispatcher->registerHandler(CLEARDISPLAYMESSAGE_ACTION,
                                              *dynamic_cast<GenericMessageHandler<ClearDisplayMessageReq, ClearDisplayMessageConf>*>(this));
            m_msg_dispatcher->registerHandler(CUSTOMERINFORMATION_ACTION,
                                              *dynamic_cast<GenericMessageHandler<CustomerInformationReq, CustomerInformationConf>*>(this));
            m_msg_dispatcher->registerHandler(GETDISPLAYMESSAGES_ACTION,
                                              *dynamic_cast<GenericMessageHandler<GetDisplayMessagesReq, GetDisplayMessagesConf>*>(this));
            m_msg_dispatcher->registerHandler(SETDISPLAYMESSAGE_ACTION,
                                              *dynamic_cast<GenericMessageHandler<SetDisplayMessageReq, SetDisplayMessageConf>*>(this));

            // Start connection
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

/** @copydoc bool IChargePoint20::stop() */
bool ChargePoint20::stop()
{
    bool ret = false;

    // Check if it is already started
    if (m_rpc_client && !m_stop_in_progress)
    {
        LOG_INFO << "Stopping OCPP stack";
        m_stop_in_progress = true;
        m_rpc_connected = false;

        // Stop uptime counter
        if (!m_stack_config.databasePath().empty())
        {
            m_uptime_timer.stop();
            saveUptime();
        }

        // Stop connection
        ret = m_rpc_client->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Free resources
        m_data_transfer_manager.reset();
        m_security_manager.reset();
        m_notify_manager.reset();
        m_meter_values_manager.reset();
        m_transaction_manager.reset();
        m_smart_charging_manager.reset();
        m_reservation_manager.reset();
        m_maintenance_manager.reset();
        m_monitoring_manager.reset();
        m_device_model_messages_manager.reset();
        m_requests_fifo_manager.reset();
        m_status_manager.reset();
        m_trigger_manager.reset();
        m_requests_fifo.reset();
        m_authent_manager.reset();
        m_ws_client.reset();
        m_rpc_client.reset();
        m_msg_dispatcher.reset();
        m_msg_sender.reset();

        // Close database
        m_database.close();
    }
    else
    {
        LOG_ERROR << "Stack already stopped";
    }

    return ret;
}

/** @copydoc bool IChargePoint20::reconnect() */
bool ChargePoint20::reconnect()
{
    bool ret = false;

    // Check if it is started
    if (m_rpc_client)
    {
        // Schedule of reconnexion
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

/** @copydoc bool IChargePoint20::dataTransfer(const std::string&,
 *                                             const std::string&,
 *                                             const std::string&,
 *                                             DataTransferStatusEnumType&,
 *                                             std::string&,
 *                                             std::string&,
 *                                             std::string&)
 */
bool ChargePoint20::dataTransfer(const std::string&                               vendor_id,
                                 const std::string&                               message_id,
                                 const std::string&                               request_data,
                                 ocpp::types::ocpp20::DataTransferStatusEnumType& status,
                                 std::string&                                     response_data,
                                 std::string&                                     error,
                                 std::string&                                     message)
{
    bool ret = false;
    if (m_data_transfer_manager)
    {
        ret = m_data_transfer_manager->dataTransfer(vendor_id, message_id, request_data, status, response_data, error, message);
    }
    else
    {
        error = "NotSupported";
        message = "DataTransfer manager is not available";
    }
    return ret;
}

/** @copydoc bool IChargePoint20::authorize(const ocpp::types::ocpp20::IdTokenType&,
 *                                          ocpp::types::ocpp20::IdTokenInfoType&,
 *                                          std::string&,
 *                                          std::string&)
 */
bool ChargePoint20::authorize(const ocpp::types::ocpp20::IdTokenType& id_token,
                              ocpp::types::ocpp20::IdTokenInfoType&   token_info,
                              std::string&                            error,
                              std::string&                            message)
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

/** @copydoc bool IChargePoint20::startTransaction(unsigned int,
 *                                                 unsigned int,
 *                                                 const ocpp::types::ocpp20::IdTokenType&,
 *                                                 ocpp::types::ocpp20::TriggerReasonEnumType,
 *                                                 std::string&)
 */
bool ChargePoint20::startTransaction(unsigned int                                evse_id,
                                     unsigned int                                connector_id,
                                     const ocpp::types::ocpp20::IdTokenType&     id_token,
                                     ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                                     std::string&                                transaction_id)
{
    return startTransaction(evse_id, connector_id, id_token, trigger_reason, -1, transaction_id);
}

/** @copydoc bool IChargePoint20::startTransaction(unsigned int,
 *                                                 unsigned int,
 *                                                 const ocpp::types::ocpp20::IdTokenType&,
 *                                                 ocpp::types::ocpp20::TriggerReasonEnumType,
 *                                                 int,
 *                                                 std::string&)
 */
bool ChargePoint20::startTransaction(unsigned int                                evse_id,
                                     unsigned int                                connector_id,
                                     const ocpp::types::ocpp20::IdTokenType&     id_token,
                                     ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
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

/** @copydoc bool IChargePoint20::startTransaction(unsigned int, unsigned int, const std::string&, std::string&) */
bool ChargePoint20::startTransaction(unsigned int evse_id, unsigned int connector_id, const std::string& id_token, std::string& transaction_id)
{
    IdTokenType token;
    token.idToken.assign(id_token);
    token.type = IdTokenEnumType::ISO14443;

    return startTransaction(evse_id, connector_id, token, TriggerReasonEnumType::Authorized, transaction_id);
}

/** @copydoc bool IChargePoint20::updateTransaction(const std::string&,
 *                                                  ocpp::types::ocpp20::TriggerReasonEnumType,
 *                                                  const std::vector<ocpp::types::ocpp20::MeterValueType>&)
 */
bool ChargePoint20::updateTransaction(const std::string&                                      transaction_id,
                                      ocpp::types::ocpp20::TriggerReasonEnumType              trigger_reason,
                                      const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->updateTransaction(transaction_id, trigger_reason, meter_values);
    }
    return ret;
}

/** @copydoc bool IChargePoint20::hasActiveTransaction(const std::string&) */
bool ChargePoint20::hasActiveTransaction(const std::string& transaction_id)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->hasActiveTransaction(transaction_id);
    }
    return ret;
}

/** @copydoc bool IChargePoint20::stopTransaction(const std::string&,
 *                                                ocpp::types::ocpp20::ReasonEnumType,
 *                                                ocpp::types::ocpp20::TriggerReasonEnumType,
 *                                                const ocpp::types::ocpp20::IdTokenType*,
 *                                                const std::vector<ocpp::types::ocpp20::MeterValueType>&)
 */
bool ChargePoint20::stopTransaction(const std::string&                                      transaction_id,
                                    ocpp::types::ocpp20::ReasonEnumType                    reason,
                                    ocpp::types::ocpp20::TriggerReasonEnumType             trigger_reason,
                                    const ocpp::types::ocpp20::IdTokenType*                id_token,
                                    const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values)
{
    bool ret = false;
    if (m_transaction_manager)
    {
        ret = m_transaction_manager->stopTransaction(transaction_id, reason, trigger_reason, id_token, meter_values);
    }
    return ret;
}

/** @copydoc bool IChargePoint20::stopTransaction(const std::string&, const std::string&, ocpp::types::ocpp20::ReasonEnumType) */
bool ChargePoint20::stopTransaction(const std::string& transaction_id, const std::string& id_token, ReasonEnumType reason)
{
    Optional<IdTokenType> token;
    if (!id_token.empty())
    {
        token.value().idToken.assign(id_token);
        token.value().type = IdTokenEnumType::ISO14443;
    }

    return stopTransaction(transaction_id, reason, TriggerReasonEnumType::StopAuthorized, token.isSet() ? &token.value() : nullptr, {});
}

/** @copydoc void IChargePoint20::startPeriodicMeterValues(unsigned int, std::chrono::seconds) */
void ChargePoint20::startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->startPeriodicMeterValues(evse_id, interval);
    }
}

/** @copydoc void IChargePoint20::stopPeriodicMeterValues(unsigned int) */
void ChargePoint20::stopPeriodicMeterValues(unsigned int evse_id)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->stopPeriodicMeterValues(evse_id);
    }
}

/** @copydoc void IChargePoint20::startTransactionSampledMeterValues(const std::string&, unsigned int, std::chrono::seconds) */
void ChargePoint20::startTransactionSampledMeterValues(const std::string& transaction_id,
                                                       unsigned int       evse_id,
                                                       std::chrono::seconds interval)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->startTransactionSampledMeterValues(transaction_id, evse_id, interval);
    }
}

/** @copydoc void IChargePoint20::stopTransactionSampledMeterValues(const std::string&) */
void ChargePoint20::stopTransactionSampledMeterValues(const std::string& transaction_id)
{
    if (m_meter_values_manager)
    {
        m_meter_values_manager->stopTransactionSampledMeterValues(transaction_id);
    }
}

/** @copydoc void IChargePoint20::getTxStopMeterValues(const std::string&,
 *                                                     std::vector<ocpp::types::ocpp20::MeterValueType>&)
 */
void ChargePoint20::getTxStopMeterValues(const std::string& transaction_id,
                                         std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values)
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

/** @copydoc bool IChargePoint20::getSetpoint(...) */
bool ChargePoint20::getSetpoint(unsigned int                                               evse_id,
                                Optional<ISmartChargingManager20::SmartChargingSetpoint>& charging_station_setpoint,
                                Optional<ISmartChargingManager20::SmartChargingSetpoint>& evse_setpoint,
                                ChargingRateUnitEnumType                                  unit)
{
    bool ret = false;
    if (m_smart_charging_manager)
    {
        ret = m_smart_charging_manager->getSetpoint(evse_id, charging_station_setpoint, evse_setpoint, unit);
    }
    return ret;
}

/** @copydoc void RpcClient::IListener::rpcClientConnected() */
void ChargePoint20::rpcClientConnected()
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
    if (m_requests_fifo_manager)
    {
        m_requests_fifo_manager->updateConnectionStatus(true);
    }
}

/** @copydoc void RpcClient::IListener::rpcClientFailed() */
void ChargePoint20::rpcClientFailed()
{
    m_rpc_connected = false;
    LOG_ERROR << "Connection failed with Central System";
    m_events_handler.connectionFailed();
}

/** @copydoc void IRpc::IListener::rpcDisconnected() */
void ChargePoint20::rpcDisconnected()
{
    // Check if stop is in progress
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
        if (m_requests_fifo_manager)
        {
            m_requests_fifo_manager->updateConnectionStatus(false);
        }
    }
}

/** @copydoc void IRpc::IListener::rpcError() */
void ChargePoint20::rpcError()
{
    LOG_ERROR << "Connection error with Central System";
}

/** @copydoc void IRpc::IListener::rpcCallReceived(const std::string&,
                                                   const rapidjson::Value&,
                                                   rapidjson::Document&,
                                                   std::string&,
                                                   std::string&) */
bool ChargePoint20::rpcCallReceived(const std::string&      action,
                                    const rapidjson::Value& payload,
                                    rapidjson::Document&    response,
                                    std::string&            error_code,
                                    std::string&            error_message)
{
    bool ret = false;

    // Check if stop is in progress
    if (!m_stop_in_progress)
    {
        ret = m_msg_dispatcher->dispatchMessage(action, payload, response, error_code, error_message);
    }

    return ret;
}

/** @copydoc void IRpc::ISpy::rcpMessageReceived(const std::string&) */
void ChargePoint20::rcpMessageReceived(const std::string& msg)
{
    LOG_COM << "RX : " << msg;
}

/** @copydoc void IRpc::ISpy::rcpMessageSent(const std::string&) */
void ChargePoint20::rcpMessageSent(const std::string& msg)
{
    LOG_COM << "TX : " << msg;
}

/** @copydoc bool IChargePoint20::statusNotification(unsigned int, unsigned int, ocpp::types::ocpp20::ConnectorStatusEnumType) */
bool ChargePoint20::statusNotification(unsigned int evse_id, unsigned int connector_id, ConnectorStatusEnumType status)
{
    StatusNotificationReq request;
    request.timestamp       = DateTime::now();
    request.connectorStatus = status;
    request.evseId          = static_cast<int>(evse_id);
    request.connectorId     = static_cast<int>(connector_id);

    StatusNotificationConf response;
    std::string            error;
    std::string            message;
    return call(STATUSNOTIFICATION_ACTION, request, response, error, message);
}

// OCPP handlers

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
bool ChargePoint20::handleMessage(const ocpp::messages::ocpp20::ClearDisplayMessageReq& request,
                                  ocpp::messages::ocpp20::ClearDisplayMessageConf&      response,
                                  std::string&                                          error_code,
                                  std::string&                                          error_message)
{
    return m_events_handler.onClearDisplayMessage(request, response, error_code, error_message);
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
bool ChargePoint20::handleMessage(const ocpp::messages::ocpp20::CustomerInformationReq& request,
                                  ocpp::messages::ocpp20::CustomerInformationConf&      response,
                                  std::string&                                          error_code,
                                  std::string&                                          error_message)
{
    return m_events_handler.onCustomerInformation(request, response, error_code, error_message);
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
bool ChargePoint20::handleMessage(const ocpp::messages::ocpp20::GetDisplayMessagesReq& request,
                                  ocpp::messages::ocpp20::GetDisplayMessagesConf&      response,
                                  std::string&                                         error_code,
                                  std::string&                                         error_message)
{
    return m_events_handler.onGetDisplayMessages(request, response, error_code, error_message);
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
bool ChargePoint20::handleMessage(const ocpp::messages::ocpp20::SetDisplayMessageReq& request,
                                  ocpp::messages::ocpp20::SetDisplayMessageConf&      response,
                                  std::string&                                        error_code,
                                  std::string&                                        error_message)
{
    return m_events_handler.onSetDisplayMessage(request, response, error_code, error_message);
}

/** @brief Initialize the database */
void ChargePoint20::initDatabase()
{
    // Initialize internal configuration
    m_internal_config.initDatabaseTable();

    // Internal keys
    if (!m_internal_config.keyExist(STACK_VERSION_KEY))
    {
        m_internal_config.createKey(STACK_VERSION_KEY, OPEN_OCPP_VERSION);
    }
    else
    {
        m_internal_config.setKey(STACK_VERSION_KEY, OPEN_OCPP_VERSION);
    }
    if (!m_internal_config.keyExist(START_DATE_KEY))
    {
        m_internal_config.createKey(START_DATE_KEY, "");
    }
    if (!m_internal_config.keyExist(UPTIME_KEY))
    {
        m_internal_config.createKey(UPTIME_KEY, "0");
    }
    if (!m_internal_config.keyExist(DISCONNECTED_TIME_KEY))
    {
        m_internal_config.createKey(DISCONNECTED_TIME_KEY, "0");
    }
    if (!m_internal_config.keyExist(TOTAL_UPTIME_KEY))
    {
        m_internal_config.createKey(TOTAL_UPTIME_KEY, "0");
    }
    else
    {
        std::string value;
        m_internal_config.getKey(TOTAL_UPTIME_KEY, value);
        m_total_uptime = static_cast<unsigned int>(std::atoi(value.c_str()));
    }
    if (!m_internal_config.keyExist(TOTAL_DISCONNECTED_TIME_KEY))
    {
        m_internal_config.createKey(TOTAL_DISCONNECTED_TIME_KEY, "0");
    }
    else
    {
        std::string value;
        m_internal_config.getKey(TOTAL_DISCONNECTED_TIME_KEY, value);
        m_total_disconnected_time = static_cast<unsigned int>(std::atoi(value.c_str()));
    }
    if (!m_internal_config.keyExist(LAST_CONNECTION_URL_KEY))
    {
        m_internal_config.createKey(LAST_CONNECTION_URL_KEY, "");
    }
}

/** @brief Process uptime */
void ChargePoint20::processUptime()
{
    // Increase counters
    m_uptime++;
    m_total_uptime++;
    if (m_rpc_client && !m_rpc_client->isConnected())
    {
        m_disconnected_time++;
        m_total_disconnected_time++;
    }

    // Save counters
    if ((m_uptime % 15u) == 0)
    {
        m_worker_pool->run<void>(std::bind(&ChargePoint20::saveUptime, this));
    }
}

/** @brief Save the uptime counter in database */
void ChargePoint20::saveUptime()
{
    m_internal_config.setKey(UPTIME_KEY, std::to_string(m_uptime));
    m_internal_config.setKey(DISCONNECTED_TIME_KEY, std::to_string(m_disconnected_time));
    m_internal_config.setKey(TOTAL_UPTIME_KEY, std::to_string(m_total_uptime));
    m_internal_config.setKey(TOTAL_DISCONNECTED_TIME_KEY, std::to_string(m_total_disconnected_time));
}

/** @brief Schedule a reconnection to the Central System */
void ChargePoint20::scheduleReconnect()
{
    // Check if a reconnection is not already scheduled
    if (!m_reconnect_scheduled)
    {
        m_reconnect_scheduled = true;
        m_worker_pool->run<void>(
            [this]
            {
                // Wait to let some time to configure other parameters
                // => Needed when switching security profiles
                std::this_thread::sleep_for(std::chrono::seconds(1u));
                doConnect();
            });
    }
}

/** @brief Start the connection process to the Central System */
bool ChargePoint20::doConnect()
{
    // Check if already connected
    if (m_rpc_client->isConnected())
    {
        // Close connection
        m_rpc_connected = false;
        m_rpc_client->stop();
    }

    // Get current security profile
    unsigned int security_profile = m_stack_config.securityProfile();

    // Build connection URL
    std::string connection_url = m_stack_config.connexionUrl();
    if (connection_url[connection_url.size() - 1] != '/')
    {
        connection_url += "/";
    }
    connection_url += ocpp::websockets::Url::encode(m_stack_config.chargePointIdentifier());

    // Configure websocket link
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

        // Use certificates prodivided by the user application
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

    // Start connection process
    m_reconnect_scheduled = false;
    return m_rpc_client->start(connection_url,
                               credentials,
                               m_stack_config.connectionTimeout(),
                               m_stack_config.retryInterval(),
                               m_stack_config.webSocketPingInterval());
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
