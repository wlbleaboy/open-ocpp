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

#ifndef OPENOCPP_OCPP21_AUTHENTLOCALLIST21_H
#define OPENOCPP_OCPP21_AUTHENTLOCALLIST21_H

#include "AuthorizationData21.h"
#include "Database.h"
#include "GenericMessageHandler.h"
#include "GetLocalListVersion21.h"
#include "IdTokenInfoType21.h"
#include "IdTokenType21.h"
#include "SendLocalList21.h"

#include <memory>
#include <string>
#include <vector>

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
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief OCPP 2.1 local authorization list */
class AuthentLocalList21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetLocalListVersionReq,
                                                   ocpp::messages::ocpp21::GetLocalListVersionConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SendLocalListReq,
                                                   ocpp::messages::ocpp21::SendLocalListConf>
{
  public:
    AuthentLocalList21(ocpp::database::Database&                       database,
                       IChargePointEventsHandler21&                    events_handler,
                       const ocpp::messages::GenericMessagesConverter& messages_converter,
                       ocpp::messages::IMessageDispatcher&             msg_dispatcher);
    virtual ~AuthentLocalList21();

    bool check(const ocpp::types::ocpp21::IdTokenType& id_token, ocpp::types::ocpp21::IdTokenInfoType& token_info);

    bool handleMessage(const ocpp::messages::ocpp21::GetLocalListVersionReq& request,
                       ocpp::messages::ocpp21::GetLocalListVersionConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SendLocalListReq& request,
                       ocpp::messages::ocpp21::SendLocalListConf&      response,
                       std::string&                                    error_code,
                       std::string&                                    error_message) override;

  private:
    static constexpr unsigned int MAX_ENTRIES_COUNT = 10000u;

    ocpp::database::Database& m_database;
    IChargePointEventsHandler21& m_events_handler;
    int m_local_list_version;
    std::unique_ptr<ocpp::database::Database::Query> m_find_query;
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    std::unique_ptr<ocpp::database::Database::Query> m_update_query;

    void initDatabaseTable();
    bool performFullUpdate(const std::vector<ocpp::types::ocpp21::AuthorizationData>& authorization_datas);
    bool performDifferentialUpdate(const std::vector<ocpp::types::ocpp21::AuthorizationData>& authorization_datas);
    void loadVersion();
    void saveVersion();
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_AUTHENTLOCALLIST21_H
