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

#include "AuthentLocalList20.h"

#include "AuthentUtils20.h"
#include "Database.h"
#include "IChargePointEventsHandler20.h"
#include "IInternalConfigManager.h"
#include "IMessageDispatcher.h"
#include "InternalConfigKeys.h"
#include "Logger.h"

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
AuthentLocalList20::AuthentLocalList20(ocpp::database::Database&                       database,
                                       IChargePointEventsHandler20&                    events_handler,
                                       ocpp::config::IInternalConfigManager&           internal_config,
                                       const ocpp::messages::GenericMessagesConverter& messages_converter,
                                       ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<GetLocalListVersionReq, GetLocalListVersionConf>(GETLOCALLISTVERSION_ACTION, messages_converter),
      GenericMessageHandler<SendLocalListReq, SendLocalListConf>(SENDLOCALLIST_ACTION, messages_converter),
      m_database(database),
      m_events_handler(events_handler),
      m_internal_config(internal_config),
      m_local_list_version(0),
      m_find_query(),
      m_delete_query(),
      m_insert_query(),
      m_update_query()
{
    initDatabaseTable();
    msg_dispatcher.registerHandler(GETLOCALLISTVERSION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetLocalListVersionReq, GetLocalListVersionConf>*>(this));
    msg_dispatcher.registerHandler(SENDLOCALLIST_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SendLocalListReq, SendLocalListConf>*>(this));

    std::string local_list_version;
    if (m_internal_config.getKey(LOCAL_LIST_VERSION_KEY, local_list_version))
    {
        m_local_list_version = std::atoi(local_list_version.c_str());
    }
}

/** @brief Destructor */
AuthentLocalList20::~AuthentLocalList20() { }

/** @brief Look for a token in the local authorization list */
bool AuthentLocalList20::check(const IdTokenType& id_token, IdTokenInfoType& token_info)
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

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
 *                                                                                ResponseType& response,
 *                                                                                std::string& error_code,
 *                                                                                std::string& error_message)
 */
bool AuthentLocalList20::handleMessage(const GetLocalListVersionReq& request,
                                       GetLocalListVersionConf&      response,
                                       std::string&                  error_code,
                                       std::string&                  error_message)
{
    (void)request;
    (void)error_code;
    (void)error_message;

    response.versionNumber = m_local_list_version;
    LOG_INFO << "Local authorization list version requested : " << response.versionNumber;

    return true;
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
 *                                                                                ResponseType& response,
 *                                                                                std::string& error_code,
 *                                                                                std::string& error_message)
 */
bool AuthentLocalList20::handleMessage(const SendLocalListReq& request,
                                       SendLocalListConf&      response,
                                       std::string&            error_code,
                                       std::string&            error_message)
{
    (void)error_code;
    (void)error_message;

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
    else
    {
        response.status = SendLocalListStatusEnumType::Failed;
    }

    LOG_INFO << "Local authorization list update status : " << SendLocalListStatusEnumTypeHelper.toString(response.status);

    return true;
}

/** @brief Initialize the database table */
void AuthentLocalList20::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS AuthentLocalList20 ("
                                  "[id] INTEGER,"
                                  "[token] VARCHAR(36),"
                                  "[type] VARCHAR(32),"
                                  "[authorization_data] TEXT,"
                                  "PRIMARY KEY([id] AUTOINCREMENT));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 authent local list table : " << query->lastError();
    }

    query = m_database.query("CREATE UNIQUE INDEX IF NOT EXISTS AuthentLocalList20TokenType ON AuthentLocalList20(token, type);");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 authent local list index : " << query->lastError();
    }

    m_find_query   = m_database.query("SELECT * FROM AuthentLocalList20 WHERE token=? AND type=?;");
    m_delete_query = m_database.query("DELETE FROM AuthentLocalList20 WHERE token=? AND type=?;");
    m_insert_query = m_database.query("INSERT INTO AuthentLocalList20 VALUES (NULL, ?, ?, ?);");
    m_update_query = m_database.query("UPDATE AuthentLocalList20 SET [authorization_data]=? WHERE id=?;");

    if (!m_internal_config.keyExist(LOCAL_LIST_VERSION_KEY))
    {
        m_internal_config.createKey(LOCAL_LIST_VERSION_KEY, std::to_string(m_local_list_version));
    }
}

/** @brief Perform a full update of the local list */
bool AuthentLocalList20::performFullUpdate(const std::vector<AuthorizationData>& authorization_datas)
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
        auto query = m_database.query("DELETE FROM AuthentLocalList20 WHERE TRUE;");
        if (query)
        {
            ret = query->exec();
            if (!ret)
            {
                LOG_ERROR << "Could not clear OCPP2 authent local list table";
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

/** @brief Perform a differential update of the local list */
bool AuthentLocalList20::performDifferentialUpdate(const std::vector<AuthorizationData>& authorization_datas)
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

/** @brief Save the current local list version */
void AuthentLocalList20::saveVersion()
{
    if (!m_internal_config.setKey(LOCAL_LIST_VERSION_KEY, std::to_string(m_local_list_version)))
    {
        LOG_ERROR << "Unable to save OCPP2 local authorization list version";
    }
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
