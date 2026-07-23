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

#ifndef OPENOCPP_OCPP21_METERVALUESMANAGER21_H
#define OPENOCPP_OCPP21_METERVALUESMANAGER21_H

#include "Database.h"
#include "GenericMessageHandler.h"
#include "ITriggerMessageManager21.h"
#include "MeterValues21.h"
#include "ReadingContextEnumType21.h"
#include "Timer.h"

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace helpers
{
class ITimerPool;
class WorkerThreadPool;
} // namespace helpers
namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;
class TransactionManager21;

/** @brief Handle OCPP 2.1 charge point MeterValues messages */
class MeterValuesManager21 : public ITriggerMessageManager21::ITriggerMessageHandler
{
  public:
    MeterValuesManager21(ocpp::database::Database&             database,
                         IChargePointEventsHandler21&          events_handler,
                         ocpp::helpers::ITimerPool&            timer_pool,
                         ocpp::helpers::WorkerThreadPool&      worker_pool,
                         ocpp::messages::GenericMessageSender& msg_sender,
                         ITriggerMessageManager21&             trigger_manager,
                         TransactionManager21&                 transaction_manager);
    virtual ~MeterValuesManager21();

    bool call(const ocpp::messages::ocpp21::MeterValuesReq& request,
              ocpp::messages::ocpp21::MeterValuesConf&      response,
              std::string&                              error,
              std::string&                              message);

    bool sendMeterValues(unsigned int evse_id, const std::vector<ocpp::types::ocpp21::MeterValueType>& values);

    void startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval);

    void stopPeriodicMeterValues(unsigned int evse_id);

    void startTransactionSampledMeterValues(const std::string& transaction_id,
                                            unsigned int       evse_id,
                                            std::chrono::seconds interval);

    void stopTransactionSampledMeterValues(const std::string& transaction_id);

    void getTxStopMeterValues(const std::string& transaction_id,
                              std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values);

    bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) override;

  private:
    struct TransactionSampleSession
    {
        std::string transaction_id;
        unsigned int evse_id;
        std::unique_ptr<ocpp::helpers::Timer> timer;
    };

    ocpp::database::Database& m_database;
    IChargePointEventsHandler21& m_events_handler;
    ocpp::helpers::ITimerPool& m_timer_pool;
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    TransactionManager21& m_transaction_manager;
    std::mutex m_mutex;
    std::map<unsigned int, std::unique_ptr<ocpp::helpers::Timer>> m_periodic_timers;
    std::map<std::string, TransactionSampleSession> m_transaction_timers;
    std::unique_ptr<ocpp::database::Database::Query> m_find_query;
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;

    void initDatabaseTable();
    bool sample(unsigned int evse_id,
                ocpp::types::ocpp21::ReadingContextEnumType context,
                ocpp::types::ocpp21::MeterValueType& meter_value);
    void processPeriodic(unsigned int evse_id);
    void processTransactionSampled(const std::string& transaction_id, unsigned int evse_id);
    bool processTriggered(unsigned int evse_id);
    void storeTxMeterValue(const std::string& transaction_id, const ocpp::types::ocpp21::MeterValueType& meter_value);
    bool serialize(const ocpp::types::ocpp21::MeterValueType& meter_value, std::string& meter_value_str);
    bool deserialize(const std::string& meter_value_str, ocpp::types::ocpp21::MeterValueType& meter_value);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_METERVALUESMANAGER21_H
