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

#include "DisplayManager21.h"

#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

DisplayManager21::DisplayManager21(IChargePointEventsHandler21&                    events_handler,
                                   const ocpp::messages::GenericMessagesConverter& messages_converter,
                                   ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<ClearDisplayMessageReq, ClearDisplayMessageConf>(CLEARDISPLAYMESSAGE_ACTION, messages_converter),
      GenericMessageHandler<GetDisplayMessagesReq, GetDisplayMessagesConf>(GETDISPLAYMESSAGES_ACTION, messages_converter),
      GenericMessageHandler<SetDisplayMessageReq, SetDisplayMessageConf>(SETDISPLAYMESSAGE_ACTION, messages_converter),
      m_events_handler(events_handler)
{
    msg_dispatcher.registerHandler(CLEARDISPLAYMESSAGE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<ClearDisplayMessageReq, ClearDisplayMessageConf>*>(this));
    msg_dispatcher.registerHandler(GETDISPLAYMESSAGES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetDisplayMessagesReq, GetDisplayMessagesConf>*>(this));
    msg_dispatcher.registerHandler(SETDISPLAYMESSAGE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetDisplayMessageReq, SetDisplayMessageConf>*>(this));
}

DisplayManager21::~DisplayManager21() { }

bool DisplayManager21::handleMessage(const ClearDisplayMessageReq& request,
                                     ClearDisplayMessageConf&      response,
                                     std::string&                  error_code,
                                     std::string&                  error_message)
{
    return m_events_handler.onClearDisplayMessage(request, response, error_code, error_message);
}

bool DisplayManager21::handleMessage(const GetDisplayMessagesReq& request,
                                     GetDisplayMessagesConf&      response,
                                     std::string&                 error_code,
                                     std::string&                 error_message)
{
    return m_events_handler.onGetDisplayMessages(request, response, error_code, error_message);
}

bool DisplayManager21::handleMessage(const SetDisplayMessageReq& request,
                                     SetDisplayMessageConf&      response,
                                     std::string&                error_code,
                                     std::string&                error_message)
{
    return m_events_handler.onSetDisplayMessage(request, response, error_code, error_message);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
