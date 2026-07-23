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

#ifndef OPENOCPP_OCPP21_AUTHENTMANAGER21_H
#define OPENOCPP_OCPP21_AUTHENTMANAGER21_H

#include "Authorize21.h"
#include "IAuthentManager21.h"

#include <memory>

namespace ocpp
{
namespace database
{
class Database;
} // namespace database
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace chargepoint
{
namespace ocpp21
{

class AuthentCache21;
class AuthentLocalList21;
class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 charge point authentication requests */
class AuthentManager21 : public IAuthentManager21
{
  public:
    AuthentManager21(ocpp::database::Database&                       database,
                     IChargePointEventsHandler21&                    events_handler,
                     const ocpp::messages::GenericMessagesConverter& messages_converter,
                     ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                     ocpp::messages::GenericMessageSender&           msg_sender);
    virtual ~AuthentManager21();

    bool call(const ocpp::messages::ocpp21::AuthorizeReq& request,
              ocpp::messages::ocpp21::AuthorizeConf&      response,
              std::string&                                error,
              std::string&                                message);

    bool authorize(const ocpp::types::ocpp21::IdTokenType& id_token,
                   ocpp::types::ocpp21::IdTokenInfoType&  token_info,
                   std::string&                           error,
                   std::string&                           message) override;

    void update(const ocpp::types::ocpp21::IdTokenType& id_token, const ocpp::types::ocpp21::IdTokenInfoType& token_info) override;

  private:
    ocpp::messages::GenericMessageSender& m_msg_sender;
    std::unique_ptr<AuthentCache21> m_cache;
    std::unique_ptr<AuthentLocalList21> m_local_list;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_AUTHENTMANAGER21_H
