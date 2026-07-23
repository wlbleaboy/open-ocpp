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

#include "AuthentManager21.h"

#include "AuthentCache21.h"
#include "AuthentLocalList21.h"
#include "GenericMessageSender.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

AuthentManager21::AuthentManager21(ocpp::database::Database&                       database,
                                   IChargePointEventsHandler21&                    events_handler,
                                   const ocpp::messages::GenericMessagesConverter& messages_converter,
                                   ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                   ocpp::messages::GenericMessageSender&           msg_sender)
    : m_msg_sender(msg_sender),
      m_cache(std::make_unique<AuthentCache21>(database, events_handler, messages_converter, msg_dispatcher)),
      m_local_list(std::make_unique<AuthentLocalList21>(database, events_handler, messages_converter, msg_dispatcher))
{
}

AuthentManager21::~AuthentManager21()
{
}

bool AuthentManager21::call(const AuthorizeReq& request, AuthorizeConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(AUTHORIZE_ACTION, request, response, error, message) == CallResult::Ok);
}

bool AuthentManager21::authorize(const IdTokenType& id_token, IdTokenInfoType& token_info, std::string& error, std::string& message)
{
    bool ret = false;

    if (m_local_list->check(id_token, token_info))
    {
        ret = true;
        LOG_DEBUG << "idToken [" << id_token.idToken.str() << "] found in local authorization list";
    }
    else if (m_cache->check(id_token, token_info))
    {
        ret = true;
        LOG_DEBUG << "idToken [" << id_token.idToken.str() << "] found in authorization cache";
    }
    else if (m_msg_sender.isConnected())
    {
        AuthorizeReq request;
        request.idToken = id_token;

        AuthorizeConf response;
        if (m_msg_sender.call(AUTHORIZE_ACTION, request, response, error, message) == CallResult::Ok)
        {
            token_info = response.idTokenInfo;
            update(id_token, token_info);
            ret = true;
        }
    }

    if (ret)
    {
        LOG_INFO << "Authorization for idToken [" << id_token.idToken.str()
                 << "] : " << AuthorizationStatusEnumTypeHelper.toString(token_info.status);
    }
    else
    {
        LOG_INFO << "Authorization for idToken [" << id_token.idToken.str() << "] : no status";
    }

    return ret;
}

void AuthentManager21::update(const IdTokenType& id_token, const IdTokenInfoType& token_info)
{
    IdTokenInfoType unused_token_info;
    if (!m_local_list->check(id_token, unused_token_info))
    {
        m_cache->update(id_token, token_info);
    }
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
