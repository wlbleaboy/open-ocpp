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

#ifndef OPENOCPP_OCPP21_TRANSACTIONMANAGER21_H
#define OPENOCPP_OCPP21_TRANSACTIONMANAGER21_H

#include "ChangeTransactionTariff21.h"
#include "GetTransactionStatus21.h"
#include "RequestBatterySwap21.h"
#include "RequestStartTransaction21.h"
#include "RequestStopTransaction21.h"
#include "BatterySwap21.h"
#include "CostUpdated21.h"
#include "TransactionEvent21.h"
#include "GenericMessageHandler.h"
#include "ITriggerMessageManager21.h"

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
class IRequestFifo;
} // namespace messages
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers
namespace chargepoint
{
namespace ocpp21
{

class IAuthentManager21;
class IChargePointEventsHandler21;
class IReservationManager21;
class MeterValuesManager21;

/** @brief Handle OCPP 2.1 charge point transaction messages */
class TransactionManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ChangeTransactionTariffReq, ocpp::messages::ocpp21::ChangeTransactionTariffConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetTransactionStatusReq, ocpp::messages::ocpp21::GetTransactionStatusConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::RequestBatterySwapReq, ocpp::messages::ocpp21::RequestBatterySwapConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::RequestStartTransactionReq, ocpp::messages::ocpp21::RequestStartTransactionConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::RequestStopTransactionReq, ocpp::messages::ocpp21::RequestStopTransactionConf>,
      public ITriggerMessageManager21::ITriggerMessageHandler
{
  public:
    TransactionManager21(IChargePointEventsHandler21&                         events_handler,
                         const ocpp::messages::GenericMessagesConverter&      messages_converter,
                         ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
                         ocpp::messages::GenericMessageSender&                msg_sender,
                         ocpp::messages::IRequestFifo&                        requests_fifo,
                         IAuthentManager21&                                   authent_manager,
                         std::chrono::seconds                                 tx_updated_interval,
                         ITriggerMessageManager21&                            trigger_manager,
                         ocpp::helpers::WorkerThreadPool&                     worker_pool,
                         IReservationManager21*                                reservation_manager = nullptr);
    virtual ~TransactionManager21();

    void setMeterValuesManager(MeterValuesManager21& meter_values_manager);

    bool call(const ocpp::messages::ocpp21::BatterySwapReq& request,
              ocpp::messages::ocpp21::BatterySwapConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::CostUpdatedReq& request,
              ocpp::messages::ocpp21::CostUpdatedConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::TransactionEventReq& request,
              ocpp::messages::ocpp21::TransactionEventConf&      response,
              std::string&                              error,
              std::string&                              message);

    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp21::IdTokenType&     id_token,
                          ocpp::types::ocpp21::TriggerReasonEnumType trigger_reason,
                          std::string&                                transaction_id);

    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp21::IdTokenType&     id_token,
                          ocpp::types::ocpp21::TriggerReasonEnumType trigger_reason,
                          int                                         remote_start_id,
                          std::string&                                transaction_id);

    bool updateTransaction(const std::string&                                      transaction_id,
                           ocpp::types::ocpp21::TriggerReasonEnumType              trigger_reason,
                           const std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values);

    bool hasActiveTransaction(const std::string& transaction_id);

    bool hasActiveTransaction(unsigned int evse_id, unsigned int connector_id);

    bool stopTransaction(const std::string&                                      transaction_id,
                         ocpp::types::ocpp21::ReasonEnumType                    reason,
                         ocpp::types::ocpp21::TriggerReasonEnumType             trigger_reason,
                         const ocpp::types::ocpp21::IdTokenType*                id_token,
                         const std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values);

    bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) override;

    bool handleMessage(const ocpp::messages::ocpp21::ChangeTransactionTariffReq& request,
                       ocpp::messages::ocpp21::ChangeTransactionTariffConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetTransactionStatusReq& request,
                       ocpp::messages::ocpp21::GetTransactionStatusConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::RequestBatterySwapReq& request,
                       ocpp::messages::ocpp21::RequestBatterySwapConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::RequestStartTransactionReq& request,
                       ocpp::messages::ocpp21::RequestStartTransactionConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::RequestStopTransactionReq& request,
                       ocpp::messages::ocpp21::RequestStopTransactionConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;

  private:
    struct Transaction
    {
        std::string transaction_id;
        unsigned int evse_id;
        unsigned int connector_id;
        int remote_start_id;
        int seq_no;
        bool active;
    };

    std::string generateTransactionId();
    bool sendTransactionEvent(Transaction& transaction,
                              ocpp::types::ocpp21::TransactionEventEnumType event_type,
                              ocpp::types::ocpp21::TriggerReasonEnumType trigger_reason,
                              const ocpp::types::ocpp21::ReasonEnumType* stopped_reason,
                              const ocpp::types::ocpp21::IdTokenType* id_token,
                              const std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values);
    void triggerTransactionEvent(const std::vector<std::string>& transaction_ids);

    IChargePointEventsHandler21& m_events_handler;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    ocpp::messages::IRequestFifo& m_requests_fifo;
    IAuthentManager21& m_authent_manager;
    IReservationManager21* m_reservation_manager;
    MeterValuesManager21* m_meter_values_manager;
    std::chrono::seconds m_tx_updated_interval;
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    std::mutex m_mutex;
    std::unordered_map<std::string, Transaction> m_transactions;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_TRANSACTIONMANAGER21_H
