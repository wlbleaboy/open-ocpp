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

#include "TransactionManager20.h"

#include "GenericMessageSender.h"
#include "IAuthentManager20.h"
#include "IChargePointEventsHandler20.h"
#include "IMeterValuesManager20.h"
#include "IMessageDispatcher.h"
#include "IReservationManager20.h"
#include "IRequestFifo.h"
#include "ISmartChargingManager20.h"
#include "Logger.h"
#include "WorkerThreadPool.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace ocpp::database;
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
TransactionManager20::TransactionManager20(ocpp::database::Database&                       database,
                                           IChargePointEventsHandler20&                    events_handler,
                                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                                           ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                           ocpp::messages::GenericMessageSender&           msg_sender,
                                           ocpp::messages::IRequestFifo&                   requests_fifo,
                                           IAuthentManager20&                              authent_manager,
                                           std::chrono::seconds                            tx_updated_interval,
                                           ITriggerMessageManager&                         trigger_manager,
                                           ocpp::helpers::WorkerThreadPool&                worker_pool,
                                           IReservationManager20*                          reservation_manager,
                                           ISmartChargingManager20*                         smart_charging_manager)
    : GenericMessageHandler<GetTransactionStatusReq, GetTransactionStatusConf>(GETTRANSACTIONSTATUS_ACTION, messages_converter),
      GenericMessageHandler<RequestStartTransactionReq, RequestStartTransactionConf>(REQUESTSTARTTRANSACTION_ACTION, messages_converter),
      GenericMessageHandler<RequestStopTransactionReq, RequestStopTransactionConf>(REQUESTSTOPTRANSACTION_ACTION, messages_converter),
      m_database(database),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_requests_fifo(requests_fifo),
      m_authent_manager(authent_manager),
      m_meter_values_manager(nullptr),
      m_tx_updated_interval(tx_updated_interval),
      m_worker_pool(worker_pool),
      m_reservation_manager(reservation_manager),
      m_smart_charging_manager(smart_charging_manager),
      m_mutex(),
      m_transactions(),
      m_find_query(),
      m_insert_query(),
      m_update_query(),
      m_update_seq_query()
{
    initDatabaseTable();
    loadTransactions();

    msg_dispatcher.registerHandler(GETTRANSACTIONSTATUS_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetTransactionStatusReq, GetTransactionStatusConf>*>(this));
    msg_dispatcher.registerHandler(REQUESTSTARTTRANSACTION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<RequestStartTransactionReq, RequestStartTransactionConf>*>(this));
    msg_dispatcher.registerHandler(REQUESTSTOPTRANSACTION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<RequestStopTransactionReq, RequestStopTransactionConf>*>(this));

    trigger_manager.registerHandler(MessageTriggerEnumType::TransactionEvent, *this);
}

/** @brief Destructor */
TransactionManager20::~TransactionManager20() { }

/** @brief Set the associated meter values manager */
void TransactionManager20::setMeterValuesManager(IMeterValuesManager20& meter_values_manager)
{
    m_meter_values_manager = &meter_values_manager;
}

/** @copydoc bool ITransactionManager20::costUpdated(...) */
bool TransactionManager20::costUpdated(const CostUpdatedReq& request,
                                       CostUpdatedConf&      response,
                                       std::string&          error,
                                       std::string&          message)
{
    bool ret = false;

    CallResult result = m_msg_sender.call(COSTUPDATED_ACTION, request, response, error, message);
    if (result == CallResult::Ok)
    {
        ret = true;
    }
    else
    {
        LOG_ERROR << "CostUpdated => " << (result == CallResult::Failed ? "Timeout" : "Error");
    }

    return ret;
}

/** @brief Start a transaction */
bool TransactionManager20::startTransaction(unsigned int                            evse_id,
                                            unsigned int                            connector_id,
                                            const IdTokenType&                      id_token,
                                            TriggerReasonEnumType                   trigger_reason,
                                            std::string&                            transaction_id)
{
    return startTransaction(evse_id, connector_id, id_token, trigger_reason, -1, transaction_id);
}

/** @brief Start a transaction with an optional remote start id */
bool TransactionManager20::startTransaction(unsigned int                            evse_id,
                                            unsigned int                            connector_id,
                                            const IdTokenType&                      id_token,
                                            TriggerReasonEnumType                   trigger_reason,
                                            int                                     remote_start_id,
                                            std::string&                            transaction_id)
{
    bool ret = false;

    IdTokenInfoType token_info;
    std::string     error;
    std::string     message;
    if (m_authent_manager.authorize(id_token, token_info, error, message) && (token_info.status == AuthorizationStatusEnumType::Accepted) &&
        ((m_reservation_manager == nullptr) || m_reservation_manager->isTransactionAllowed(evse_id, id_token)))
    {
        Transaction transaction;
        transaction.transaction_id = generateTransactionId();
        transaction.evse_id        = evse_id;
        transaction.connector_id   = connector_id;
        transaction.remote_start_id = remote_start_id;
        transaction.seq_no         = 0;
        transaction.id_token       = id_token.idToken.str();
        transaction.id_token_type  = IdTokenEnumTypeHelper.toString(id_token.type);
        transaction.started_at     = DateTime::now();
        transaction.active         = true;

        ret = sendTransactionEvent(transaction, TransactionEventEnumType::Started, trigger_reason, nullptr, &id_token, {});
        if (ret)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                insertTransaction(transaction);
                m_transactions[transaction.transaction_id] = transaction;
                transaction_id                             = transaction.transaction_id;
            }
            if (m_reservation_manager)
            {
                m_reservation_manager->transactionStarted(evse_id, id_token);
            }
            if (m_meter_values_manager)
            {
                m_meter_values_manager->startTransactionSampledMeterValues(transaction_id, evse_id, m_tx_updated_interval);
            }
        }
    }

    return ret;
}

/** @brief Send a transaction update */
bool TransactionManager20::updateTransaction(const std::string& transaction_id,
                                             TriggerReasonEnumType trigger_reason,
                                             const std::vector<MeterValueType>& meter_values)
{
    bool ret = false;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto                        transaction = m_transactions.find(transaction_id);
    if (transaction != m_transactions.end() && transaction->second.active)
    {
        ret = sendTransactionEvent(transaction->second, TransactionEventEnumType::Updated, trigger_reason, nullptr, nullptr, meter_values);
        saveTransaction(transaction->second);
    }

    return ret;
}

/** @brief Stop a transaction */
bool TransactionManager20::stopTransaction(const std::string& transaction_id,
                                           ReasonEnumType reason,
                                           TriggerReasonEnumType trigger_reason,
                                           const IdTokenType* id_token,
                                           const std::vector<MeterValueType>& meter_values)
{
    bool ret = false;

    std::vector<MeterValueType> tx_stop_meter_values = meter_values;
    if (m_meter_values_manager)
    {
        m_meter_values_manager->stopTransactionSampledMeterValues(transaction_id);

        std::vector<MeterValueType> stored_meter_values;
        m_meter_values_manager->getTxStopMeterValues(transaction_id, stored_meter_values);
        tx_stop_meter_values.insert(tx_stop_meter_values.end(), stored_meter_values.begin(), stored_meter_values.end());
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto                        transaction = m_transactions.find(transaction_id);
    if (transaction != m_transactions.end() && transaction->second.active)
    {
        ret = sendTransactionEvent(transaction->second, TransactionEventEnumType::Ended, trigger_reason, &reason, id_token, tx_stop_meter_values);
        if (ret)
        {
            transaction->second.active = false;
            saveTransaction(transaction->second);
            m_transactions.erase(transaction);
        }
    }

    return ret;
}

/** @copydoc bool ITriggerMessageManager::ITriggerMessageHandler::onTriggerMessage(...) */
bool TransactionManager20::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
{
    bool ret = false;

    if (message == MessageTriggerEnumType::TransactionEvent)
    {
        std::vector<std::string> transaction_ids;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& transaction : m_transactions)
            {
                if (transaction.second.active &&
                    (!evse.isSet() || (transaction.second.evse_id == static_cast<unsigned int>(evse.value().id))))
                {
                    transaction_ids.push_back(transaction.first);
                }
            }
        }

        ret = !transaction_ids.empty();
        if (ret)
        {
            m_worker_pool.run<void>(
                [this, transaction_ids]
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250u));
                    triggerTransactionEvent(transaction_ids);
                });
        }
    }

    return ret;
}

/** @brief Handle GetTransactionStatus */
bool TransactionManager20::handleMessage(const GetTransactionStatusReq& request,
                                         GetTransactionStatusConf&      response,
                                         std::string&                   error_code,
                                         std::string&                   error_message)
{
    (void)error_code;
    (void)error_message;

    std::lock_guard<std::mutex> lock(m_mutex);

    if (request.transactionId.isSet())
    {
        response.ongoingIndicator = (m_transactions.find(request.transactionId.value().str()) != m_transactions.end());
    }
    else
    {
        response.ongoingIndicator = !m_transactions.empty();
    }
    response.messagesInQueue = false;

    return true;
}

/** @brief Handle RequestStartTransaction */
bool TransactionManager20::handleMessage(const RequestStartTransactionReq& request,
                                         RequestStartTransactionConf&      response,
                                         std::string&                      error_code,
                                         std::string&                      error_message)
{
    (void)error_code;
    (void)error_message;

    IdTokenInfoType token_info;
    std::string     error;
    std::string     message;
    const unsigned int evse_id = static_cast<unsigned int>(request.evseId.isSet() ? request.evseId.value() : 1);
    if (m_authent_manager.authorize(request.idToken, token_info, error, message) &&
        (token_info.status == AuthorizationStatusEnumType::Accepted))
    {
        bool authorized = ((m_reservation_manager == nullptr) || m_reservation_manager->isTransactionAllowed(evse_id, request.idToken));
        if (authorized)
        {
            authorized = m_events_handler.remoteStartTransactionRequested(evse_id, request.remoteStartId, request.idToken);
            if (authorized && request.chargingProfile.isSet() && m_smart_charging_manager)
            {
                authorized = m_smart_charging_manager->installTxProfile(evse_id, request.chargingProfile.value());
            }
        }

        if (authorized)
        {
            response.status = RequestStartStopStatusEnumType::Accepted;
        }
        else
        {
            response.status = RequestStartStopStatusEnumType::Rejected;
        }
    }
    else
    {
        response.status = RequestStartStopStatusEnumType::Rejected;
    }

    return true;
}

/** @brief Handle RequestStopTransaction */
bool TransactionManager20::handleMessage(const RequestStopTransactionReq& request,
                                         RequestStopTransactionConf&      response,
                                         std::string&                     error_code,
                                         std::string&                     error_message)
{
    (void)error_code;
    (void)error_message;

    if (m_events_handler.remoteStopTransactionRequested(request.transactionId.str()))
    {
        response.status = RequestStartStopStatusEnumType::Accepted;
    }
    else
    {
        response.status = RequestStartStopStatusEnumType::Rejected;
    }

    return true;
}

/** @brief Initialize database */
void TransactionManager20::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS Transactions20 ("
                                  "[id] INTEGER,"
                                  "[transaction_id] VARCHAR(36),"
                                  "[evse_id] INTEGER,"
                                  "[connector_id] INTEGER,"
                                  "[remote_start_id] INTEGER,"
                                  "[seq_no] INTEGER,"
                                  "[id_token] VARCHAR(36),"
                                  "[id_token_type] VARCHAR(32),"
                                  "[started_at] INTEGER,"
                                  "[active] INTEGER,"
                                  "PRIMARY KEY([id] AUTOINCREMENT));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 transactions table : " << query->lastError();
    }

    query = m_database.query("CREATE UNIQUE INDEX IF NOT EXISTS Transactions20TransactionId ON Transactions20(transaction_id);");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 transactions index : " << query->lastError();
    }

    m_find_query       = m_database.query("SELECT * FROM Transactions20 WHERE active=1;");
    m_insert_query     = m_database.query("INSERT INTO Transactions20 VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    m_update_query     = m_database.query("UPDATE Transactions20 SET [seq_no]=?, [active]=? WHERE transaction_id=?;");
    m_update_seq_query = m_database.query("UPDATE Transactions20 SET [seq_no]=? WHERE transaction_id=?;");
}

/** @brief Load active transactions */
void TransactionManager20::loadTransactions()
{
    if (m_find_query && m_find_query->exec())
    {
        while (m_find_query->hasRows())
        {
            Transaction transaction;
            transaction.transaction_id  = m_find_query->getString(1);
            transaction.evse_id         = static_cast<unsigned int>(m_find_query->getInt32(2));
            transaction.connector_id    = static_cast<unsigned int>(m_find_query->getInt32(3));
            transaction.remote_start_id = m_find_query->getInt32(4);
            transaction.seq_no          = m_find_query->getInt32(5);
            transaction.id_token        = m_find_query->getString(6);
            transaction.id_token_type   = m_find_query->getString(7);
            transaction.started_at      = DateTime(m_find_query->getInt64(8));
            transaction.active          = m_find_query->getBool(9);
            m_transactions[transaction.transaction_id] = transaction;

            if (!m_find_query->next())
            {
                break;
            }
        }
        m_find_query->reset();
    }
}

/** @brief Save a started transaction */
void TransactionManager20::insertTransaction(const Transaction& transaction)
{
    if (m_insert_query)
    {
        m_insert_query->bind(0, transaction.transaction_id);
        m_insert_query->bind(1, static_cast<int32_t>(transaction.evse_id));
        m_insert_query->bind(2, static_cast<int32_t>(transaction.connector_id));
        m_insert_query->bind(3, transaction.remote_start_id);
        m_insert_query->bind(4, transaction.seq_no);
        m_insert_query->bind(5, transaction.id_token);
        m_insert_query->bind(6, transaction.id_token_type);
        m_insert_query->bind(7, static_cast<int64_t>(transaction.started_at.timestamp()));
        m_insert_query->bind(8, transaction.active);
        if (!m_insert_query->exec())
        {
            LOG_ERROR << "Could not insert OCPP2 transaction [" << transaction.transaction_id << "]";
        }
        m_insert_query->reset();
    }
}

/** @brief Save a transaction update */
void TransactionManager20::saveTransaction(const Transaction& transaction)
{
    if (m_update_query)
    {
        m_update_query->bind(0, transaction.seq_no);
        m_update_query->bind(1, transaction.active);
        m_update_query->bind(2, transaction.transaction_id);
        if (!m_update_query->exec())
        {
            LOG_ERROR << "Could not update OCPP2 transaction [" << transaction.transaction_id << "]";
        }
        m_update_query->reset();
    }
}

/** @brief Generate a transaction id */
std::string TransactionManager20::generateTransactionId()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    std::stringstream id;
    id << "tx-" << std::hex << ms << "-" << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(std::rand());
    return id.str().substr(0, 36);
}

/** @brief Send a transaction event */
bool TransactionManager20::sendTransactionEvent(Transaction& transaction,
                                                TransactionEventEnumType event_type,
                                                TriggerReasonEnumType trigger_reason,
                                                ReasonEnumType* stopped_reason,
                                                const IdTokenType* id_token,
                                                const std::vector<MeterValueType>& meter_values)
{
    bool ret = false;

    TransactionEventReq request;
    request.eventType     = event_type;
    request.timestamp     = DateTime::now();
    request.triggerReason = trigger_reason;
    request.seqNo         = transaction.seq_no++;
    request.meterValue    = meter_values;

    request.transactionInfo.transactionId.assign(transaction.transaction_id);
    if (transaction.remote_start_id >= 0)
    {
        request.transactionInfo.remoteStartId = transaction.remote_start_id;
    }
    if (stopped_reason)
    {
        request.transactionInfo.stoppedReason = *stopped_reason;
    }
    else
    {
        request.transactionInfo.chargingState = ChargingStateEnumType::EVConnected;
    }

    EVSEType evse;
    evse.id = static_cast<int>(transaction.evse_id);
    evse.connectorId = static_cast<int>(transaction.connector_id);
    request.evse     = evse;

    if (id_token)
    {
        request.idToken = *id_token;
    }
    if (!m_msg_sender.isConnected())
    {
        request.offline = true;
    }

    std::string          error;
    std::string          message;
    TransactionEventConf response;
    CallResult result = m_msg_sender.call(TRANSACTIONEVENT_ACTION, request, response, error, message, &m_requests_fifo, transaction.evse_id);
    if (result == CallResult::Ok)
    {
        if (response.idTokenInfo.isSet() && id_token)
        {
            m_authent_manager.update(*id_token, response.idTokenInfo.value());
        }
        ret = true;
    }
    else if (result == CallResult::Delayed)
    {
        ret = true;
    }
    else
    {
        --transaction.seq_no;
        LOG_WARNING << "TransactionEvent failed for transaction [" << transaction.transaction_id << "] : " << error << " - " << message;
    }

    return ret;
}

/** @brief Send triggered transaction event updates */
void TransactionManager20::triggerTransactionEvent(const std::vector<std::string>& transaction_ids)
{
    for (const std::string& transaction_id : transaction_ids)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        transaction = m_transactions.find(transaction_id);
        if (transaction != m_transactions.end() && transaction->second.active)
        {
            if (sendTransactionEvent(
                    transaction->second, TransactionEventEnumType::Updated, TriggerReasonEnumType::Trigger, nullptr, nullptr, {}))
            {
                saveTransaction(transaction->second);
            }
        }
    }
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
