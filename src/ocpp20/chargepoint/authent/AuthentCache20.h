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

#ifndef OPENOCPP_OCPP20_AUTHENTCACHE20_H
#define OPENOCPP_OCPP20_AUTHENTCACHE20_H

#include "ClearCache20.h"
#include "Database.h"
#include "GenericMessageHandler.h"
#include "IdTokenInfoType20.h"
#include "IdTokenType20.h"

#include <memory>
#include <string>

namespace ocpp
{
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

/** @brief OCPP 2.0.1 local authorization cache */
class AuthentCache20 : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ClearCacheReq,
                                                                    ocpp::messages::ocpp20::ClearCacheConf>
{
  public:
    /** @brief Constructor */
    AuthentCache20(ocpp::database::Database&                       database,
                   IChargePointEventsHandler20&                    events_handler,
                   const ocpp::messages::GenericMessagesConverter& messages_converter,
                   ocpp::messages::IMessageDispatcher&             msg_dispatcher);

    /** @brief Destructor */
    virtual ~AuthentCache20();

    /** @brief Look for a token in the cache */
    bool check(const ocpp::types::ocpp20::IdTokenType& id_token, ocpp::types::ocpp20::IdTokenInfoType& token_info);

    /** @brief Update a token in the cache */
    void update(const ocpp::types::ocpp20::IdTokenType& id_token, const ocpp::types::ocpp20::IdTokenInfoType& token_info);

    /** @brief Clear all cached tokens */
    void clear();

    // GenericMessageHandler interface

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::ClearCacheReq& request,
                       ocpp::messages::ocpp20::ClearCacheConf&      response,
                       std::string&                                 error_code,
                       std::string&                                 error_message) override;

  private:
    /** @brief Maximum number of cache entries */
    static constexpr unsigned int MAX_ENTRIES_COUNT = 1000u;

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
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
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_AUTHENTCACHE20_H
