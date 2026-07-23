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

#include "MeterValuesManager21.h"

#include "DateTime.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "ITimerPool.h"
#include "Logger.h"
#include "TransactionManager21.h"
#include "WorkerThreadPool.h"

#include <functional>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace ocpp::database;
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

MeterValuesManager21::MeterValuesManager21(ocpp::database::Database&             database,
                                           IChargePointEventsHandler21&          events_handler,
                                           ocpp::helpers::ITimerPool&            timer_pool,
                                           ocpp::helpers::WorkerThreadPool&      worker_pool,
                                           ocpp::messages::GenericMessageSender& msg_sender,
                                           ITriggerMessageManager21&             trigger_manager,
                                           TransactionManager21&                 transaction_manager)
    : m_database(database),
      m_events_handler(events_handler),
      m_timer_pool(timer_pool),
      m_worker_pool(worker_pool),
      m_msg_sender(msg_sender),
      m_transaction_manager(transaction_manager),
      m_mutex(),
      m_periodic_timers(),
      m_transaction_timers(),
      m_find_query(),
      m_delete_query(),
      m_insert_query()
{
    initDatabaseTable();
    trigger_manager.registerHandler(MessageTriggerEnumType::MeterValues, *this);
}

MeterValuesManager21::~MeterValuesManager21()
{
    for (auto& timer : m_periodic_timers)
    {
        timer.second->stop();
    }
    for (auto& timer : m_transaction_timers)
    {
        timer.second.timer->stop();
    }
}

bool MeterValuesManager21::call(const MeterValuesReq& request, MeterValuesConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(METERVALUES_ACTION, request, response, error, message) == CallResult::Ok);
}

bool MeterValuesManager21::sendMeterValues(unsigned int evse_id, const std::vector<MeterValueType>& values)
{
    bool ret = false;

    if (!values.empty())
    {
        MeterValuesReq request;
        request.evseId     = static_cast<int>(evse_id);
        request.meterValue = values;

        MeterValuesConf response;
        CallResult      result = m_msg_sender.call(METERVALUES_ACTION, request, response);
        ret                    = (result == CallResult::Ok);
    }

    return ret;
}

void MeterValuesManager21::startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto current_timer = m_periodic_timers.find(evse_id);
    if (current_timer != m_periodic_timers.end())
    {
        current_timer->second->stop();
        m_periodic_timers.erase(current_timer);
    }
    if (interval > std::chrono::seconds(0))
    {
        std::unique_ptr<ocpp::helpers::Timer> timer(m_timer_pool.createTimer("OCPP2.1 MeterValues periodic"));
        timer->setCallback([this, evse_id] { m_worker_pool.run<void>(std::bind(&MeterValuesManager21::processPeriodic, this, evse_id)); });
        timer->start(interval);
        m_periodic_timers[evse_id] = std::move(timer);
        LOG_INFO << "OCPP2.1 periodic MeterValues started : EVSE = " << evse_id << " - interval = " << interval.count() << "s";
    }
}

void MeterValuesManager21::stopPeriodicMeterValues(unsigned int evse_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto timer = m_periodic_timers.find(evse_id);
    if (timer != m_periodic_timers.end())
    {
        timer->second->stop();
        m_periodic_timers.erase(timer);
        LOG_INFO << "OCPP2.1 periodic MeterValues stopped : EVSE = " << evse_id;
    }
}

void MeterValuesManager21::startTransactionSampledMeterValues(const std::string& transaction_id,
                                                             unsigned int       evse_id,
                                                             std::chrono::seconds interval)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto current_timer = m_transaction_timers.find(transaction_id);
    if (current_timer != m_transaction_timers.end())
    {
        current_timer->second.timer->stop();
        m_transaction_timers.erase(current_timer);
    }
    if (!transaction_id.empty() && (interval > std::chrono::seconds(0)))
    {
        TransactionSampleSession session;
        session.transaction_id = transaction_id;
        session.evse_id        = evse_id;
        session.timer.reset(m_timer_pool.createTimer("OCPP2.1 transaction MeterValues"));
        session.timer->setCallback([this, transaction_id, evse_id] {
            m_worker_pool.run<void>(std::bind(&MeterValuesManager21::processTransactionSampled, this, transaction_id, evse_id));
        });
        session.timer->start(interval);
        m_transaction_timers[transaction_id] = std::move(session);
        LOG_INFO << "OCPP2.1 transaction MeterValues started : transactionId = " << transaction_id << " - interval = " << interval.count()
                 << "s";
    }
}

void MeterValuesManager21::stopTransactionSampledMeterValues(const std::string& transaction_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto timer = m_transaction_timers.find(transaction_id);
    if (timer != m_transaction_timers.end())
    {
        timer->second.timer->stop();
        m_transaction_timers.erase(timer);
        LOG_INFO << "OCPP2.1 transaction MeterValues stopped : transactionId = " << transaction_id;
    }
}

void MeterValuesManager21::getTxStopMeterValues(const std::string& transaction_id, std::vector<MeterValueType>& meter_values)
{
    meter_values.clear();

    if (m_find_query && m_delete_query)
    {
        m_find_query->bind(0, transaction_id);
        if (m_find_query->exec() && m_find_query->hasRows())
        {
            MeterValueType meter_value;
            if (deserialize(m_find_query->getString(2), meter_value))
            {
                meter_values.push_back(meter_value);
            }
        }
        m_find_query->reset();

        m_delete_query->bind(0, transaction_id);
        m_delete_query->exec();
        m_delete_query->reset();
    }
}

bool MeterValuesManager21::onTriggerMessage(MessageTriggerEnumType message, const Optional<EVSEType>& evse)
{
    bool ret = false;

    if ((message == MessageTriggerEnumType::MeterValues) && evse.isSet())
    {
        const unsigned int evse_id = static_cast<unsigned int>(evse.value().id);
        ret                        = processTriggered(evse_id);
    }
    else
    {
        LOG_WARNING << "Triggered MeterValues requires an EVSE";
    }

    return ret;
}

void MeterValuesManager21::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS TxMeterValues21 ("
                                  "[id] INTEGER,"
                                  "[transaction_id] VARCHAR(36),"
                                  "[meter_value] TEXT,"
                                  "PRIMARY KEY([id] AUTOINCREMENT));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 transaction meter values table : " << query->lastError();
    }

    m_find_query   = m_database.query("SELECT * FROM TxMeterValues21 WHERE transaction_id=? ORDER BY id DESC LIMIT 1;");
    m_delete_query = m_database.query("DELETE FROM TxMeterValues21 WHERE transaction_id=?;");
    m_insert_query = m_database.query("INSERT INTO TxMeterValues21 VALUES (NULL, ?, ?);");
}

bool MeterValuesManager21::sample(unsigned int evse_id, ReadingContextEnumType context, MeterValueType& meter_value)
{
    meter_value.sampledValue.clear();
    meter_value.timestamp = DateTime::now();

    bool ret = m_events_handler.getMeterValue(evse_id, context, meter_value);
    if (!ret)
    {
        LOG_DEBUG << "No OCPP2.1 meter value sampled from events handler : EVSE = " << evse_id;
    }

    if (ret)
    {
        for (SampledValueType& sampled_value : meter_value.sampledValue)
        {
            sampled_value.context = context;
        }
    }

    return (ret && !meter_value.sampledValue.empty());
}

void MeterValuesManager21::processPeriodic(unsigned int evse_id)
{
    MeterValueType meter_value;
    if (sample(evse_id, ReadingContextEnumType::Sample_Periodic, meter_value))
    {
        sendMeterValues(evse_id, {meter_value});
    }
}

void MeterValuesManager21::processTransactionSampled(const std::string& transaction_id, unsigned int evse_id)
{
    MeterValueType meter_value;
    if (sample(evse_id, ReadingContextEnumType::Sample_Periodic, meter_value))
    {
        storeTxMeterValue(transaction_id, meter_value);
        m_transaction_manager.updateTransaction(transaction_id, TriggerReasonEnumType::MeterValuePeriodic, {meter_value});
    }
}

bool MeterValuesManager21::processTriggered(unsigned int evse_id)
{
    bool ret = false;

    MeterValueType meter_value;
    if (sample(evse_id, ReadingContextEnumType::Trigger, meter_value))
    {
        ret = sendMeterValues(evse_id, {meter_value});
        if (ret)
        {
            LOG_INFO << "Triggered MeterValues sent : EVSE = " << evse_id;
        }
        else
        {
            LOG_WARNING << "Triggered MeterValues couldn't be sent : EVSE = " << evse_id;
        }
    }
    else
    {
        LOG_WARNING << "Triggered MeterValues couldn't be sampled : EVSE = " << evse_id;
    }

    return ret;
}

void MeterValuesManager21::storeTxMeterValue(const std::string& transaction_id, const MeterValueType& meter_value)
{
    if (m_insert_query)
    {
        std::string meter_value_str;
        if (serialize(meter_value, meter_value_str))
        {
            m_insert_query->bind(0, transaction_id);
            m_insert_query->bind(1, meter_value_str);
            m_insert_query->exec();
            m_insert_query->reset();
        }
    }
}

bool MeterValuesManager21::serialize(const MeterValueType& meter_value, std::string& meter_value_str)
{
    bool                    ret = false;
    MeterValueTypeConverter converter;
    rapidjson::Document     doc;
    doc.SetObject();
    converter.setAllocator(&doc.GetAllocator());
    if (converter.toJson(meter_value, doc))
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        meter_value_str = buffer.GetString();
        ret             = true;
    }
    return ret;
}

bool MeterValuesManager21::deserialize(const std::string& meter_value_str, MeterValueType& meter_value)
{
    bool                ret = false;
    rapidjson::Document doc;
    if (!doc.Parse(meter_value_str.c_str()).HasParseError() && doc.IsObject())
    {
        std::string             error_code;
        std::string             error_message;
        MeterValueTypeConverter converter;
        ret = converter.fromJson(doc, meter_value, error_code, error_message);
        if (!ret)
        {
            LOG_ERROR << "Unable to parse OCPP2.1 transaction meter value : " << error_code << " - " << error_message;
        }
    }
    return ret;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
