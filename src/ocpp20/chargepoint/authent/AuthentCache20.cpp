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

#include "AuthentCache20.h"

#include "AuthentUtils20.h"
#include "Database.h"
#include "IChargePointEventsHandler20.h"
#include "IMessageDispatcher.h"
#include "Logger.h"

#include <sstream>

using namespace ocpp::database;
using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;
using namespace ocpp::types::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
AuthentCache20::AuthentCache20(ocpp::database::Database&                       database,
                               IChargePointEventsHandler20&                    events_handler,
                               const ocpp::messages::GenericMessagesConverter& messages_converter,
                               ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<ClearCacheReq, ClearCacheConf>(CLEARCACHE_ACTION, messages_converter),
      m_database(database),
      m_events_handler(events_handler),
      m_find_query(),
      m_delete_query(),
      m_insert_query(),
      m_update_query()
{
    initDatabaseTable();
    msg_dispatcher.registerHandler(CLEARCACHE_ACTION, *this);
}

/** @brief Destructor */
AuthentCache20::~AuthentCache20() { }

/** @brief Look for a token in the cache */
bool AuthentCache20::check(const IdTokenType& id_token, IdTokenInfoType& token_info)
{
    bool ret = false;

    if (m_find_query)
    {
        m_find_query->bind(0, tokenKey(id_token));
        m_find_query->bind(1, tokenType(id_token));
        if (m_find_query->exec() && m_find_query->hasRows())
        {
            ret = deserializeIdTokenInfo(m_find_query->getString(3), token_info);
            if (ret && isExpired(token_info))
            {
                if (m_delete_query)
                {
                    m_delete_query->bind(0, tokenKey(id_token));
                    m_delete_query->bind(1, tokenType(id_token));
                    m_delete_query->exec();
                    m_delete_query->reset();
                }
                ret = false;
            }
        }
        m_find_query->reset();
    }
    return ret;
}

/** @brief Update a token in the cache */
void AuthentCache20::update(const IdTokenType& id_token, const IdTokenInfoType& token_info)
{
    if (m_find_query)
    {
        std::string token_info_json;
        if (serializeIdTokenInfo(token_info, token_info_json))
        {
            m_find_query->bind(0, tokenKey(id_token));
            m_find_query->bind(1, tokenType(id_token));
            if (m_find_query->exec())
            {
                if (m_find_query->hasRows())
                {
                    if (m_update_query)
                    {
                        m_update_query->bind(0, token_info_json);
                        m_update_query->bind(1, m_find_query->getInt32(0));
                        if (!m_update_query->exec())
                        {
                            LOG_ERROR << "Could not update cached idToken [" << tokenKey(id_token) << "]";
                        }
                        m_update_query->reset();
                    }
                }
                else
                {
                    if (m_insert_query)
                    {
                        m_insert_query->bind(0, tokenKey(id_token));
                        m_insert_query->bind(1, tokenType(id_token));
                        m_insert_query->bind(2, token_info_json);
                        if (!m_insert_query->exec())
                        {
                            LOG_ERROR << "Could not insert cached idToken [" << tokenKey(id_token) << "]";
                        }
                        m_insert_query->reset();
                    }
                }
            }
            m_find_query->reset();
        }
    }
}

/** @brief Clear all cached tokens */
void AuthentCache20::clear()
{
    auto query = m_database.query("DELETE FROM AuthentCache20 WHERE TRUE;");
    if (query)
    {
        query->exec();
    }
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
 *                                                                                ResponseType& response,
 *                                                                                std::string& error_code,
 *                                                                                std::string& error_message)
 */
bool AuthentCache20::handleMessage(const ClearCacheReq& request, ClearCacheConf& response, std::string& error_code, std::string& error_message)
{
    (void)request;
    (void)error_code;
    (void)error_message;

    LOG_INFO << "Clear cache requested";

    clear();
    response.status = ClearCacheStatusEnumType::Accepted;

    LOG_INFO << "Clear cache status : " << ClearCacheStatusEnumTypeHelper.toString(response.status);

    return true;
}

/** @brief Initialize the database table */
void AuthentCache20::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS AuthentCache20 ("
                                  "[id] INTEGER,"
                                  "[token] VARCHAR(36),"
                                  "[type] VARCHAR(32),"
                                  "[token_info] TEXT,"
                                  "PRIMARY KEY([id] AUTOINCREMENT));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 authent cache table : " << query->lastError();
    }

    std::stringstream trigger_query;
    trigger_query << "CREATE TRIGGER IF NOT EXISTS delete_oldest_AuthentCache20 AFTER INSERT ON AuthentCache20 WHEN "
                     "((SELECT count() FROM AuthentCache20) > ";
    trigger_query << MAX_ENTRIES_COUNT;
    trigger_query << ") BEGIN DELETE FROM AuthentCache20 WHERE ROWID IN (SELECT ROWID FROM AuthentCache20 LIMIT 1);END;";
    query = m_database.query(trigger_query.str());
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 authent cache trigger : " << query->lastError();
    }

    query = m_database.query("CREATE UNIQUE INDEX IF NOT EXISTS AuthentCache20TokenType ON AuthentCache20(token, type);");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 authent cache index : " << query->lastError();
    }

    m_find_query   = m_database.query("SELECT * FROM AuthentCache20 WHERE token=? AND type=?;");
    m_delete_query = m_database.query("DELETE FROM AuthentCache20 WHERE token=? AND type=?;");
    m_insert_query = m_database.query("INSERT INTO AuthentCache20 VALUES (NULL, ?, ?, ?);");
    m_update_query = m_database.query("UPDATE AuthentCache20 SET [token_info]=? WHERE id=?;");
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
