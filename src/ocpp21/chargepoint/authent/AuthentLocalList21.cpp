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

#include "AuthentLocalList21.h"

#include "AuthentUtils21.h"
#include "Database.h"
#include "IMessageDispatcher.h"
#include "Logger.h"

using namespace ocpp::database;
using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

AuthentLocalList21::AuthentLocalList21(ocpp::database::Database&                       database,
                                       IChargePointEventsHandler21&                    events_handler,
                                       const ocpp::messages::GenericMessagesConverter& messages_converter,
                                       ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<GetLocalListVersionReq, GetLocalListVersionConf>(GETLOCALLISTVERSION_ACTION, messages_converter),
      GenericMessageHandler<SendLocalListReq, SendLocalListConf>(SENDLOCALLIST_ACTION, messages_converter),
      m_database(database),
      m_events_handler(events_handler),
      m_local_list_version(0),
      m_find_query(),
      m_delete_query(),
      m_insert_query(),
      m_update_query()
{
    initDatabaseTable();
    loadVersion();

    msg_dispatcher.registerHandler(GETLOCALLISTVERSION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetLocalListVersionReq, GetLocalListVersionConf>*>(this));
    msg_dispatcher.registerHandler(SENDLOCALLIST_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SendLocalListReq, SendLocalListConf>*>(this));
}

AuthentLocalList21::~AuthentLocalList21()
{
}

bool AuthentLocalList21::check(const IdTokenType& id_token, IdTokenInfoType& token_info)
{
    bool ret = false;

    if (m_find_query)
    {
        m_find_query->bind(0, tokenKey(id_token));
        m_find_query->bind(1, tokenType(id_token));
        if (m_find_query->exec() && m_find_query->hasRows())
        {
            AuthorizationData authorization_data;
            ret = deserializeAuthorizationData(m_find_query->getString(3), authorization_data);
            if (ret && authorization_data.idTokenInfo.isSet())
            {
                token_info = authorization_data.idTokenInfo.value();
                if (isExpired(token_info))
                {
                    ret = false;
                }
            }
            else
            {
                ret = false;
            }
        }
        m_find_query->reset();
    }
    return ret;
}

bool AuthentLocalList21::handleMessage(const GetLocalListVersionReq& request,
                                       GetLocalListVersionConf&      response,
                                       std::string&                  error_code,
                                       std::string&                  error_message)
{
    (void)request;
    (void)error_code;
    (void)error_message;
    (void)m_events_handler;

    response.versionNumber = m_local_list_version;
    LOG_INFO << "Local authorization list version requested : " << response.versionNumber;

    return true;
}

bool AuthentLocalList21::handleMessage(const SendLocalListReq& request,
                                       SendLocalListConf&      response,
                                       std::string&            error_code,
                                       std::string&            error_message)
{
    (void)error_code;
    (void)error_message;
    (void)m_events_handler;

    LOG_INFO << "Local authorization list update requested : versionNumber = " << request.versionNumber
             << " - updateType = " << UpdateEnumTypeHelper.toString(request.updateType);

    response.status = SendLocalListStatusEnumType::Failed;

    bool success = false;
    if ((request.versionNumber >= 0) && (request.localAuthorizationList.size() <= MAX_ENTRIES_COUNT))
    {
        if (request.updateType == UpdateEnumType::Full)
        {
            success = performFullUpdate(request.localAuthorizationList);
        }
        else if (request.versionNumber > m_local_list_version)
        {
            success = performDifferentialUpdate(request.localAuthorizationList);
        }
        else
        {
            response.status = SendLocalListStatusEnumType::VersionMismatch;
        }

        if (success)
        {
            m_local_list_version = request.versionNumber;
            saveVersion();
            response.status = SendLocalListStatusEnumType::Accepted;
        }
        else if (response.status != SendLocalListStatusEnumType::VersionMismatch)
        {
            response.status = SendLocalListStatusEnumType::Failed;
        }
    }

    LOG_INFO << "Local authorization list update status : " << SendLocalListStatusEnumTypeHelper.toString(response.status);

    return true;
}

void AuthentLocalList21::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS AuthentLocalList21 ("
                                  "[id] INTEGER,"
                                  "[token] VARCHAR(255),"
                                  "[type] VARCHAR(20),"
                                  "[authorization_data] TEXT,"
                                  "PRIMARY KEY([id] AUTOINCREMENT));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 authent local list table : " << query->lastError();
    }

    query = m_database.query("CREATE UNIQUE INDEX IF NOT EXISTS AuthentLocalList21TokenType ON AuthentLocalList21(token, type);");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 authent local list index : " << query->lastError();
    }

    query = m_database.query("CREATE TABLE IF NOT EXISTS AuthentLocalListVersion21 ("
                             "[id] INTEGER,"
                             "[version] INTEGER,"
                             "PRIMARY KEY([id]));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 authent local list version table : " << query->lastError();
    }

    m_find_query   = m_database.query("SELECT * FROM AuthentLocalList21 WHERE token=? AND type=?;");
    m_delete_query = m_database.query("DELETE FROM AuthentLocalList21 WHERE token=? AND type=?;");
    m_insert_query = m_database.query("INSERT INTO AuthentLocalList21 VALUES (NULL, ?, ?, ?);");
    m_update_query = m_database.query("UPDATE AuthentLocalList21 SET [authorization_data]=? WHERE id=?;");
}

bool AuthentLocalList21::performFullUpdate(const std::vector<AuthorizationData>& authorization_datas)
{
    bool ret = true;

    for (const AuthorizationData& authorization_data : authorization_datas)
    {
        if (!authorization_data.idTokenInfo.isSet())
        {
            LOG_ERROR << "idTokenInfo field is mandatory when performing a full local authorization list update";
            ret = false;
        }
    }

    if (ret)
    {
        auto query = m_database.query("DELETE FROM AuthentLocalList21 WHERE TRUE;");
        if (query)
        {
            ret = query->exec();
            if (!ret)
            {
                LOG_ERROR << "Could not clear OCPP2.1 authent local list table";
            }
        }
    }

    if (ret && m_insert_query)
    {
        for (const AuthorizationData& authorization_data : authorization_datas)
        {
            std::string authorization_data_json;
            if (serializeAuthorizationData(authorization_data, authorization_data_json))
            {
                m_insert_query->bind(0, tokenKey(authorization_data.idToken));
                m_insert_query->bind(1, tokenType(authorization_data.idToken));
                m_insert_query->bind(2, authorization_data_json);
                if (!m_insert_query->exec())
                {
                    LOG_ERROR << "Could not insert idToken [" << tokenKey(authorization_data.idToken) << "] in local authorization list";
                    ret = false;
                }
                m_insert_query->reset();
            }
            else
            {
                ret = false;
            }
        }
    }

    return ret;
}

bool AuthentLocalList21::performDifferentialUpdate(const std::vector<AuthorizationData>& authorization_datas)
{
    bool ret = true;

    if (m_delete_query && m_find_query && m_update_query && m_insert_query)
    {
        for (const AuthorizationData& authorization_data : authorization_datas)
        {
            if (!authorization_data.idTokenInfo.isSet())
            {
                m_delete_query->bind(0, tokenKey(authorization_data.idToken));
                m_delete_query->bind(1, tokenType(authorization_data.idToken));
                if (!m_delete_query->exec())
                {
                    LOG_ERROR << "Could not delete idToken [" << tokenKey(authorization_data.idToken) << "] from local authorization list";
                    ret = false;
                }
                m_delete_query->reset();
            }
            else
            {
                std::string authorization_data_json;
                if (serializeAuthorizationData(authorization_data, authorization_data_json))
                {
                    m_find_query->bind(0, tokenKey(authorization_data.idToken));
                    m_find_query->bind(1, tokenType(authorization_data.idToken));
                    if (m_find_query->exec())
                    {
                        if (m_find_query->hasRows())
                        {
                            m_update_query->bind(0, authorization_data_json);
                            m_update_query->bind(1, m_find_query->getInt32(0));
                            if (!m_update_query->exec())
                            {
                                LOG_ERROR << "Could not update idToken [" << tokenKey(authorization_data.idToken)
                                          << "] in local authorization list";
                                ret = false;
                            }
                            m_update_query->reset();
                        }
                        else
                        {
                            m_insert_query->bind(0, tokenKey(authorization_data.idToken));
                            m_insert_query->bind(1, tokenType(authorization_data.idToken));
                            m_insert_query->bind(2, authorization_data_json);
                            if (!m_insert_query->exec())
                            {
                                LOG_ERROR << "Could not insert idToken [" << tokenKey(authorization_data.idToken)
                                          << "] in local authorization list";
                                ret = false;
                            }
                            m_insert_query->reset();
                        }
                    }
                    else
                    {
                        ret = false;
                    }
                    m_find_query->reset();
                }
                else
                {
                    ret = false;
                }
            }
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}

void AuthentLocalList21::loadVersion()
{
    auto query = m_database.query("SELECT version FROM AuthentLocalListVersion21 WHERE id=1;");
    if (query && query->exec() && query->hasRows())
    {
        m_local_list_version = query->getInt32(0);
    }
    else
    {
        saveVersion();
    }
}

void AuthentLocalList21::saveVersion()
{
    auto query = m_database.query("INSERT OR REPLACE INTO AuthentLocalListVersion21(id, version) VALUES(1, ?);");
    if (query)
    {
        query->bind(0, m_local_list_version);
        if (!query->exec())
        {
            LOG_ERROR << "Unable to save OCPP2.1 local authorization list version";
        }
    }
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
