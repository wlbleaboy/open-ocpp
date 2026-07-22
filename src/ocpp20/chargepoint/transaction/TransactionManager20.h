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

#ifndef OPENOCPP_OCPP20_TRANSACTIONMANAGER20_H
#define OPENOCPP_OCPP20_TRANSACTIONMANAGER20_H

#include "Database.h"
#include "GenericMessageHandler.h"
#include "GetTransactionStatus20.h"
#include "ITriggerMessageManager20.h"
#include "ITransactionManager20.h"
#include "RequestStartTransaction20.h"
#include "RequestStopTransaction20.h"
#include "TransactionEvent20.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ocpp
{
namespace database
{
class Database;
}
namespace messages
{
class GenericMessageSender;
class GenericMessagesConverter;
class IMessageDispatcher;
class IRequestFifo;
} // namespace messages
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp20
{

class IAuthentManager20;
class IChargePointEventsHandler20;
class IMeterValuesManager20;
class IReservationManager20;
class ISmartChargingManager20;

/** @brief Handle OCPP 2.0.1 charge point transaction events */
class TransactionManager20
    : public ITransactionManager20,
      public ITriggerMessageManager::ITriggerMessageHandler,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetTransactionStatusReq,
                                                   ocpp::messages::ocpp20::GetTransactionStatusConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::RequestStartTransactionReq,
                                                   ocpp::messages::ocpp20::RequestStartTransactionConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::RequestStopTransactionReq,
                                                   ocpp::messages::ocpp20::RequestStopTransactionConf>
{
  public:
    /** @brief Constructor */
    TransactionManager20(ocpp::database::Database&                       database,
                         IChargePointEventsHandler20&                    events_handler,
                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                         ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                         ocpp::messages::GenericMessageSender&           msg_sender,
                         ocpp::messages::IRequestFifo&                   requests_fifo,
                         IAuthentManager20&                              authent_manager,
                         std::chrono::seconds                            tx_updated_interval,
                         ITriggerMessageManager&                         trigger_manager,
                         ocpp::helpers::WorkerThreadPool&                worker_pool,
                         IReservationManager20*                          reservation_manager = nullptr,
                         ISmartChargingManager20*                         smart_charging_manager = nullptr);

    /** @brief Destructor */
    virtual ~TransactionManager20();

    /** @brief Set the associated meter values manager */
    void setMeterValuesManager(IMeterValuesManager20& meter_values_manager);

    // ITransactionManager20 interface

    bool costUpdated(const ocpp::messages::ocpp20::CostUpdatedReq& request,
                     ocpp::messages::ocpp20::CostUpdatedConf&      response,
                     std::string&                                  error,
                     std::string&                                  message) override;

    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp20::IdTokenType&     id_token,
                          ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                          std::string&                                transaction_id) override;

    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp20::IdTokenType&     id_token,
                          ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                          int                                         remote_start_id,
                          std::string&                                transaction_id) override;

    bool updateTransaction(const std::string&                                      transaction_id,
                           ocpp::types::ocpp20::TriggerReasonEnumType              trigger_reason,
                           const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    bool stopTransaction(const std::string&                                      transaction_id,
                         ocpp::types::ocpp20::ReasonEnumType                    reason,
                         ocpp::types::ocpp20::TriggerReasonEnumType             trigger_reason,
                         const ocpp::types::ocpp20::IdTokenType*                id_token,
                         const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    // ITriggerMessageManager::ITriggerMessageHandler interface

    bool onTriggerMessage(ocpp::types::ocpp20::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp20::EVSEType>& evse) override;

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::GetTransactionStatusReq& request,
                       ocpp::messages::ocpp20::GetTransactionStatusConf&      response,
                       std::string&                                           error_code,
                       std::string&                                           error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::RequestStartTransactionReq& request,
                       ocpp::messages::ocpp20::RequestStartTransactionConf&      response,
                       std::string&                                              error_code,
                       std::string&                                              error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::RequestStopTransactionReq& request,
                       ocpp::messages::ocpp20::RequestStopTransactionConf&      response,
                       std::string&                                             error_code,
                       std::string&                                             error_message) override;

  private:
    /** @brief Transaction data */
    struct Transaction
    {
        std::string transaction_id;
        unsigned int evse_id;
        unsigned int connector_id;
        int remote_start_id;
        int seq_no;
        std::string id_token;
        std::string id_token_type;
        ocpp::types::DateTime started_at;
        bool active;
    };

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Offline request FIFO */
    ocpp::messages::IRequestFifo& m_requests_fifo;
    /** @brief Authentication manager */
    IAuthentManager20& m_authent_manager;
    /** @brief Meter values manager */
    IMeterValuesManager20* m_meter_values_manager;
    /** @brief Transaction updated sampled meter values interval */
    std::chrono::seconds m_tx_updated_interval;
    /** @brief Worker thread pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    /** @brief Reservation manager */
    IReservationManager20* m_reservation_manager;
    /** @brief Smart charging manager */
    ISmartChargingManager20* m_smart_charging_manager;
    /** @brief Transactions mutex */
    std::mutex m_mutex;
    /** @brief Active transactions */
    std::unordered_map<std::string, Transaction> m_transactions;
    /** @brief Find active transaction query */
    std::unique_ptr<ocpp::database::Database::Query> m_find_query;
    /** @brief Insert transaction query */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    /** @brief Update transaction query */
    std::unique_ptr<ocpp::database::Database::Query> m_update_query;
    /** @brief Update transaction sequence number query */
    std::unique_ptr<ocpp::database::Database::Query> m_update_seq_query;

    /** @brief Initialize database */
    void initDatabaseTable();
    /** @brief Load active transactions */
    void loadTransactions();
    /** @brief Save a started transaction */
    void insertTransaction(const Transaction& transaction);
    /** @brief Save a transaction update */
    void saveTransaction(const Transaction& transaction);
    /** @brief Generate a transaction id */
    std::string generateTransactionId();
    /** @brief Send a transaction event */
    bool sendTransactionEvent(Transaction&                                          transaction,
                              ocpp::types::ocpp20::TransactionEventEnumType        event_type,
                              ocpp::types::ocpp20::TriggerReasonEnumType           trigger_reason,
                              ocpp::types::ocpp20::ReasonEnumType*                 stopped_reason,
                              const ocpp::types::ocpp20::IdTokenType*              id_token,
                              const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values);
    /** @brief Send triggered transaction event updates */
    void triggerTransactionEvent(const std::vector<std::string>& transaction_ids);
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_TRANSACTIONMANAGER20_H
