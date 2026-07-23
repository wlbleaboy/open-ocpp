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

#include "TransactionManager21.h"
#include "DateTime.h"
#include "GenericMessageSender.h"
#include "IAuthentManager21.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"
#include "IReservationManager21.h"
#include "Logger.h"
#include "MeterValuesManager21.h"
#include "WorkerThreadPool.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <sstream>
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

TransactionManager21::TransactionManager21(IChargePointEventsHandler21&                    events_handler,
                                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                                           ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                           ocpp::messages::GenericMessageSender&           msg_sender,
                                           ocpp::messages::IRequestFifo&                   requests_fifo,
                                           IAuthentManager21&                              authent_manager,
                                           std::chrono::seconds                            tx_updated_interval,
                                           ITriggerMessageManager21&                       trigger_manager,
                                           ocpp::helpers::WorkerThreadPool&                worker_pool,
                                           IReservationManager21*                           reservation_manager)
    : GenericMessageHandler<ChangeTransactionTariffReq, ChangeTransactionTariffConf>(CHANGETRANSACTIONTARIFF_ACTION, messages_converter),
      GenericMessageHandler<GetTransactionStatusReq, GetTransactionStatusConf>(GETTRANSACTIONSTATUS_ACTION, messages_converter),
      GenericMessageHandler<RequestBatterySwapReq, RequestBatterySwapConf>(REQUESTBATTERYSWAP_ACTION, messages_converter),
      GenericMessageHandler<RequestStartTransactionReq, RequestStartTransactionConf>(REQUESTSTARTTRANSACTION_ACTION, messages_converter),
      GenericMessageHandler<RequestStopTransactionReq, RequestStopTransactionConf>(REQUESTSTOPTRANSACTION_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_requests_fifo(requests_fifo),
      m_authent_manager(authent_manager),
      m_reservation_manager(reservation_manager),
      m_meter_values_manager(nullptr),
      m_tx_updated_interval(tx_updated_interval),
      m_worker_pool(worker_pool),
      m_mutex(),
      m_transactions()
{
    msg_dispatcher.registerHandler(CHANGETRANSACTIONTARIFF_ACTION, *dynamic_cast<GenericMessageHandler<ChangeTransactionTariffReq, ChangeTransactionTariffConf>*>(this));
    msg_dispatcher.registerHandler(GETTRANSACTIONSTATUS_ACTION, *dynamic_cast<GenericMessageHandler<GetTransactionStatusReq, GetTransactionStatusConf>*>(this));
    msg_dispatcher.registerHandler(REQUESTBATTERYSWAP_ACTION, *dynamic_cast<GenericMessageHandler<RequestBatterySwapReq, RequestBatterySwapConf>*>(this));
    msg_dispatcher.registerHandler(REQUESTSTARTTRANSACTION_ACTION, *dynamic_cast<GenericMessageHandler<RequestStartTransactionReq, RequestStartTransactionConf>*>(this));
    msg_dispatcher.registerHandler(REQUESTSTOPTRANSACTION_ACTION, *dynamic_cast<GenericMessageHandler<RequestStopTransactionReq, RequestStopTransactionConf>*>(this));

    trigger_manager.registerHandler(MessageTriggerEnumType::TransactionEvent, *this);
}

TransactionManager21::~TransactionManager21()
{
}

void TransactionManager21::setMeterValuesManager(MeterValuesManager21& meter_values_manager)
{
    m_meter_values_manager = &meter_values_manager;
}

bool TransactionManager21::call(const BatterySwapReq& request, BatterySwapConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(BATTERYSWAP_ACTION, request, response, error, message) == CallResult::Ok);
}

bool TransactionManager21::call(const CostUpdatedReq& request, CostUpdatedConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(COSTUPDATED_ACTION, request, response, error, message) == CallResult::Ok);
}


bool TransactionManager21::call(const TransactionEventReq& request, TransactionEventConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(TRANSACTIONEVENT_ACTION, request, response, error, message) == CallResult::Ok);
}

bool TransactionManager21::startTransaction(unsigned int                                evse_id,
                                            unsigned int                                connector_id,
                                            const IdTokenType&                          id_token,
                                            TriggerReasonEnumType                       trigger_reason,
                                            std::string&                                transaction_id)
{
    return startTransaction(evse_id, connector_id, id_token, trigger_reason, -1, transaction_id);
}

bool TransactionManager21::startTransaction(unsigned int                                evse_id,
                                            unsigned int                                connector_id,
                                            const IdTokenType&                          id_token,
                                            TriggerReasonEnumType                       trigger_reason,
                                            int                                         remote_start_id,
                                            std::string&                                transaction_id)
{
    IdTokenInfoType token_info;
    std::string     error;
    std::string     message;
    if (!m_authent_manager.authorize(id_token, token_info, error, message) ||
        (token_info.status != AuthorizationStatusEnumType::Accepted))
    {
        return false;
    }
    if (m_reservation_manager && !m_reservation_manager->isTransactionAllowed(evse_id, id_token))
    {
        LOG_WARNING << "Transaction start rejected by reservation manager for EVSE [" << evse_id << "]";
        return false;
    }

    Transaction transaction;
    transaction.transaction_id  = generateTransactionId();
    transaction.evse_id         = evse_id;
    transaction.connector_id    = connector_id;
    transaction.remote_start_id = remote_start_id;
    transaction.seq_no          = 0;
    transaction.active          = true;

    bool ret = sendTransactionEvent(transaction, TransactionEventEnumType::Started, trigger_reason, nullptr, &id_token, {});
    if (ret)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_transactions[transaction.transaction_id] = transaction;
        transaction_id                             = transaction.transaction_id;
    }
    if (ret && m_reservation_manager)
    {
        m_reservation_manager->transactionStarted(evse_id, id_token);
    }
    if (ret && m_meter_values_manager)
    {
        m_meter_values_manager->startTransactionSampledMeterValues(transaction_id, evse_id, m_tx_updated_interval);
    }

    return ret;
}

bool TransactionManager21::updateTransaction(const std::string& transaction_id,
                                             TriggerReasonEnumType trigger_reason,
                                             const std::vector<MeterValueType>& meter_values)
{
    bool ret = false;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto                        transaction = m_transactions.find(transaction_id);
    if (transaction != m_transactions.end() && transaction->second.active)
    {
        ret = sendTransactionEvent(transaction->second, TransactionEventEnumType::Updated, trigger_reason, nullptr, nullptr, meter_values);
    }

    return ret;
}

bool TransactionManager21::hasActiveTransaction(const std::string& transaction_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto                        transaction = m_transactions.find(transaction_id);
    return (transaction != m_transactions.end()) && transaction->second.active;
}

bool TransactionManager21::hasActiveTransaction(unsigned int evse_id, unsigned int connector_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& transaction : m_transactions)
    {
        if (transaction.second.active && (transaction.second.evse_id == evse_id) &&
            (transaction.second.connector_id == connector_id))
        {
            return true;
        }
    }
    return false;
}

bool TransactionManager21::stopTransaction(const std::string& transaction_id,
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
            m_transactions.erase(transaction);
        }
    }
    else
    {
        LOG_WARNING << "Unable to stop unknown or inactive transaction [" << transaction_id << "]";
    }

    return ret;
}

bool TransactionManager21::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
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

bool TransactionManager21::handleMessage(const ChangeTransactionTariffReq& request,
                                         ChangeTransactionTariffConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    return m_events_handler.onChangeTransactionTariff(request, response, error_code, error_message);
}

bool TransactionManager21::handleMessage(const GetTransactionStatusReq& request,
                                         GetTransactionStatusConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    return m_events_handler.onGetTransactionStatus(request, response, error_code, error_message);
}

bool TransactionManager21::handleMessage(const RequestBatterySwapReq& request,
                                         RequestBatterySwapConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    return m_events_handler.onRequestBatterySwap(request, response, error_code, error_message);
}

bool TransactionManager21::handleMessage(const RequestStartTransactionReq& request,
                                         RequestStartTransactionConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    return m_events_handler.onRequestStartTransaction(request, response, error_code, error_message);
}

bool TransactionManager21::handleMessage(const RequestStopTransactionReq& request,
                                         RequestStopTransactionConf&      response,
                                         std::string&    error_code,
                                         std::string&    error_message)
{
    return m_events_handler.onRequestStopTransaction(request, response, error_code, error_message);
}

std::string TransactionManager21::generateTransactionId()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    std::stringstream id;
    id << "tx-" << std::hex << ms << "-" << std::setw(8) << std::setfill('0') << static_cast<unsigned int>(std::rand());
    return id.str().substr(0, 36);
}

bool TransactionManager21::sendTransactionEvent(Transaction& transaction,
                                                TransactionEventEnumType event_type,
                                                TriggerReasonEnumType trigger_reason,
                                                const ReasonEnumType* stopped_reason,
                                                const IdTokenType* id_token,
                                                const std::vector<MeterValueType>& meter_values)
{
    TransactionEventReq request;
    request.eventType     = event_type;
    request.timestamp     = ocpp::types::DateTime::now();
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
    evse.id          = static_cast<int>(transaction.evse_id);
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
    CallResult           result = m_msg_sender.call(TRANSACTIONEVENT_ACTION, request, response, error, message, &m_requests_fifo, transaction.evse_id);
    if (result == CallResult::Ok)
    {
        if (response.idTokenInfo.isSet() && id_token)
        {
            m_authent_manager.update(*id_token, response.idTokenInfo.value());
        }
    }
    else if (result == CallResult::Delayed)
    {
        return true;
    }
    else
    {
        --transaction.seq_no;
        LOG_WARNING << "TransactionEvent failed for transaction [" << transaction.transaction_id << "] : " << error << " - " << message;
    }

    return (result == CallResult::Ok);
}

void TransactionManager21::triggerTransactionEvent(const std::vector<std::string>& transaction_ids)
{
    for (const std::string& transaction_id : transaction_ids)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        transaction = m_transactions.find(transaction_id);
        if (transaction != m_transactions.end() && transaction->second.active)
        {
            sendTransactionEvent(transaction->second, TransactionEventEnumType::Updated, TriggerReasonEnumType::Trigger, nullptr, nullptr, {});
        }
    }
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
