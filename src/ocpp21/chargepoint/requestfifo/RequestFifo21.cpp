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

#include "RequestFifo21.h"

#include "Logger.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>

using namespace ocpp::database;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

RequestFifo21::RequestFifo21(ocpp::database::Database& database)
    : m_database(database), m_delete_query(), m_insert_query(), m_mutex(), m_fifo(), m_id(0), m_listener(nullptr)
{
    initDatabaseTable();
}

RequestFifo21::~RequestFifo21() { }

void RequestFifo21::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS RequestFifo21 ("
                                  "[id] INT UNSIGNED,"
                                  "[connector_id] INT UNSIGNED,"
                                  "[action] VARCHAR(64),"
                                  "[request] TEXT,"
                                  "PRIMARY KEY([id]));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 request FIFO table : " << query->lastError();
    }

    std::stringstream trigger_query;
    trigger_query << "CREATE TRIGGER IF NOT EXISTS delete_oldest_RequestFifo21 AFTER INSERT ON RequestFifo21 WHEN "
                     "((SELECT count() FROM RequestFifo21) > ";
    trigger_query << MAX_ENTRIES_COUNT;
    trigger_query << ") BEGIN DELETE FROM RequestFifo21 WHERE ROWID IN (SELECT ROWID FROM RequestFifo21 LIMIT 1);END;";
    query = m_database.query(trigger_query.str());
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 request FIFO trigger : " << query->lastError();
    }

    m_delete_query = m_database.query("DELETE FROM RequestFifo21 WHERE id=?;");
    m_insert_query = m_database.query("INSERT INTO RequestFifo21 VALUES (?, ?, ?, ?);");

    load();
}

void RequestFifo21::push(unsigned int connector_id, const std::string& action, const rapidjson::Document& payload)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    rapidjson::StringBuffer                    buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    payload.Accept(writer);
    std::string request = buffer.GetString();

    LOG_DEBUG << "OCPP2.1 request FIFO : pushing " << action << " request";

    m_fifo.emplace(m_id, connector_id, action, request);
    if (m_insert_query)
    {
        m_insert_query->bind(0, m_id);
        m_insert_query->bind(1, connector_id);
        m_insert_query->bind(2, action);
        m_insert_query->bind(3, request);
        m_insert_query->exec();
        m_insert_query->reset();
    }
    ++m_id;

    if (m_listener)
    {
        m_listener->requestQueued();
    }
}

bool RequestFifo21::front(unsigned int& connector_id, std::string& action, rapidjson::Document& payload)
{
    bool ret = false;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_fifo.empty())
    {
        const Entry& entry = m_fifo.front();
        connector_id      = entry.connector_id;
        action            = entry.action;
        ret               = (!payload.Parse(entry.request.c_str()).HasParseError() && payload.IsObject());
    }

    return ret;
}

void RequestFifo21::pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_fifo.empty())
    {
        LOG_DEBUG << "OCPP2.1 request FIFO : popping " << m_fifo.front().action << " request";

        unsigned int id = m_fifo.front().id;
        m_fifo.pop();
        if (m_delete_query)
        {
            m_delete_query->bind(0, id);
            m_delete_query->exec();
            m_delete_query->reset();
        }
    }
}

size_t RequestFifo21::size() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fifo.size();
}

void RequestFifo21::load()
{
    while (!m_fifo.empty())
    {
        m_fifo.pop();
    }

    auto query = m_database.query("SELECT * FROM RequestFifo21 WHERE TRUE ORDER BY id ASC;");
    if (query && query->exec() && query->hasRows())
    {
        do
        {
            unsigned int id           = query->getUInt32(0);
            unsigned int connector_id = query->getUInt32(1);
            std::string  action       = query->getString(2);
            std::string  request      = query->getString(3);
            m_fifo.emplace(id, connector_id, action, request);
        } while (query->next());

        m_id = m_fifo.back().id + 1u;
    }

    LOG_INFO << "OCPP2.1 request FIFO : " << m_fifo.size() << " message(s) pending";
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
