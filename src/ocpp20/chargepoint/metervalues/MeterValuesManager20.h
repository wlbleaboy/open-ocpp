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

#ifndef OPENOCPP_OCPP20_METERVALUESMANAGER20_H
#define OPENOCPP_OCPP20_METERVALUESMANAGER20_H

#include "Database.h"
#include "IMeterValuesManager20.h"
#include "ITriggerMessageManager20.h"
#include "MeterValues20.h"
#include "Timer.h"

#include <map>
#include <memory>
#include <mutex>

namespace ocpp
{
namespace helpers
{
class ITimerPool;
class WorkerThreadPool;
} // namespace helpers
namespace messages
{
class GenericMessageSender;
class IRequestFifo;
} // namespace messages

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;
class ITransactionManager20;

/** @brief Handle OCPP 2.0.1 charge point meter values */
class MeterValuesManager20 : public IMeterValuesManager20, public ITriggerMessageManager::ITriggerMessageHandler
{
  public:
    /** @brief Constructor */
    MeterValuesManager20(ocpp::database::Database&             database,
                         IChargePointEventsHandler20&          events_handler,
                         ocpp::helpers::ITimerPool&            timer_pool,
                         ocpp::helpers::WorkerThreadPool&      worker_pool,
                         ocpp::messages::GenericMessageSender& msg_sender,
                         ocpp::messages::IRequestFifo&         requests_fifo,
                         ITriggerMessageManager&               trigger_manager,
                         ITransactionManager20&                transaction_manager);

    /** @brief Destructor */
    virtual ~MeterValuesManager20();

    // IMeterValuesManager20 interface

    /** @copydoc bool IMeterValuesManager20::sendMeterValues(unsigned int, const std::vector<ocpp::types::ocpp20::MeterValueType>&) */
    bool sendMeterValues(unsigned int evse_id, const std::vector<ocpp::types::ocpp20::MeterValueType>& values) override;

    /** @copydoc void IMeterValuesManager20::startPeriodicMeterValues(unsigned int, std::chrono::seconds) */
    void startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval) override;

    /** @copydoc void IMeterValuesManager20::stopPeriodicMeterValues(unsigned int) */
    void stopPeriodicMeterValues(unsigned int evse_id) override;

    /** @copydoc void IMeterValuesManager20::startTransactionSampledMeterValues(const std::string&, unsigned int, std::chrono::seconds) */
    void startTransactionSampledMeterValues(const std::string& transaction_id,
                                            unsigned int       evse_id,
                                            std::chrono::seconds interval) override;

    /** @copydoc void IMeterValuesManager20::stopTransactionSampledMeterValues(const std::string&) */
    void stopTransactionSampledMeterValues(const std::string& transaction_id) override;

    /** @copydoc void IMeterValuesManager20::getTxStopMeterValues(const std::string&,
     *                                                           std::vector<ocpp::types::ocpp20::MeterValueType>&)
     */
    void getTxStopMeterValues(const std::string& transaction_id,
                              std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    // ITriggerMessageManager::ITriggerMessageHandler interface

    /** @copydoc bool ITriggerMessageManager::ITriggerMessageHandler::onTriggerMessage(ocpp::types::ocpp20::MessageTriggerEnumType,
     *                                                                                 const ocpp::types::Optional<ocpp::types::ocpp20::EVSEType>&)
     */
    bool onTriggerMessage(ocpp::types::ocpp20::MessageTriggerEnumType                 message,
                          const ocpp::types::Optional<ocpp::types::ocpp20::EVSEType>& evse) override;

  private:
    /** @brief Transaction sampling session */
    struct TransactionSampleSession
    {
        std::string transaction_id;
        unsigned int evse_id;
        std::unique_ptr<ocpp::helpers::Timer> timer;
    };

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Timer pool */
    ocpp::helpers::ITimerPool& m_timer_pool;
    /** @brief Worker thread pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Offline requests FIFO */
    ocpp::messages::IRequestFifo& m_requests_fifo;
    /** @brief Transaction manager */
    ITransactionManager20& m_transaction_manager;
    /** @brief Mutex */
    std::mutex m_mutex;
    /** @brief Periodic MeterValues timers by EVSE */
    std::map<unsigned int, std::unique_ptr<ocpp::helpers::Timer>> m_periodic_timers;
    /** @brief Transaction sampled timers by transaction id */
    std::map<std::string, TransactionSampleSession> m_transaction_timers;
    /** @brief Find transaction meter value query */
    std::unique_ptr<ocpp::database::Database::Query> m_find_query;
    /** @brief Delete transaction meter value query */
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    /** @brief Insert transaction meter value query */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;

    /** @brief Initialize database */
    void initDatabaseTable();
    /** @brief Sample a meter value from the provider */
    bool sample(unsigned int evse_id,
                ocpp::types::ocpp20::ReadingContextEnumType context,
                ocpp::types::ocpp20::MeterValueType& meter_value);
    /** @brief Process periodic MeterValues */
    void processPeriodic(unsigned int evse_id);
    /** @brief Process transaction sampled MeterValues */
    void processTransactionSampled(const std::string& transaction_id, unsigned int evse_id);
    /** @brief Process triggered MeterValues */
    bool processTriggered(unsigned int evse_id);
    /** @brief Store a transaction meter value */
    void storeTxMeterValue(const std::string& transaction_id, const ocpp::types::ocpp20::MeterValueType& meter_value);
    /** @brief Serialize a meter value */
    bool serialize(const ocpp::types::ocpp20::MeterValueType& meter_value, std::string& meter_value_str);
    /** @brief Deserialize a meter value */
    bool deserialize(const std::string& meter_value_str, ocpp::types::ocpp20::MeterValueType& meter_value);
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_METERVALUESMANAGER20_H
