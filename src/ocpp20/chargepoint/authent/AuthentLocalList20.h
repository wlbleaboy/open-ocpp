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

#ifndef OPENOCPP_OCPP20_AUTHENTLOCALLIST20_H
#define OPENOCPP_OCPP20_AUTHENTLOCALLIST20_H

#include "AuthorizationData20.h"
#include "Database.h"
#include "GenericMessageHandler.h"
#include "GetLocalListVersion20.h"
#include "IdTokenInfoType20.h"
#include "IdTokenType20.h"
#include "SendLocalList20.h"

#include <memory>
#include <string>
#include <vector>

namespace ocpp
{
namespace config
{
class IInternalConfigManager;
} // namespace config
namespace database
{
class Database;
} // namespace database
namespace messages
{
class GenericMessagesConverter;
class IMessageDispatcher;
} // namespace messages

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief OCPP 2.0.1 local authorization list */
class AuthentLocalList20
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetLocalListVersionReq,
                                                   ocpp::messages::ocpp20::GetLocalListVersionConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SendLocalListReq,
                                                   ocpp::messages::ocpp20::SendLocalListConf>
{
  public:
    /** @brief Constructor */
    AuthentLocalList20(ocpp::database::Database&                       database,
                       IChargePointEventsHandler20&                    events_handler,
                       ocpp::config::IInternalConfigManager&           internal_config,
                       const ocpp::messages::GenericMessagesConverter& messages_converter,
                       ocpp::messages::IMessageDispatcher&             msg_dispatcher);

    /** @brief Destructor */
    virtual ~AuthentLocalList20();

    /** @brief Look for a token in the local authorization list */
    bool check(const ocpp::types::ocpp20::IdTokenType& id_token, ocpp::types::ocpp20::IdTokenInfoType& token_info);

    // GenericMessageHandler interface

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::GetLocalListVersionReq& request,
                       ocpp::messages::ocpp20::GetLocalListVersionConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::SendLocalListReq& request,
                       ocpp::messages::ocpp20::SendLocalListConf&      response,
                       std::string&                                    error_code,
                       std::string&                                    error_message) override;

  private:
    /** @brief Maximum number of local authorization list entries */
    static constexpr unsigned int MAX_ENTRIES_COUNT = 10000u;

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Internal configuration */
    ocpp::config::IInternalConfigManager& m_internal_config;
    /** @brief Current local list version */
    int m_local_list_version;
    /** @brief Find query */
    std::unique_ptr<ocpp::database::Database::Query> m_find_query;
    /** @brief Delete query */
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    /** @brief Insert query */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    /** @brief Update query */
    std::unique_ptr<ocpp::database::Database::Query> m_update_query;

    /** @brief Initialize the database table */
    void initDatabaseTable();
    /** @brief Perform a full update of the local list */
    bool performFullUpdate(const std::vector<ocpp::types::ocpp20::AuthorizationData>& authorization_datas);
    /** @brief Perform a differential update of the local list */
    bool performDifferentialUpdate(const std::vector<ocpp::types::ocpp20::AuthorizationData>& authorization_datas);
    /** @brief Save the current local list version */
    void saveVersion();
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_AUTHENTLOCALLIST20_H
