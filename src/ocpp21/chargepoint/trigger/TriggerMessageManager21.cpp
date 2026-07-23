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

#include "TriggerMessageManager21.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types;
using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

TriggerMessageManager21::TriggerMessageManager21(IChargePointEventsHandler21&                    events_handler,
             const ocpp::messages::GenericMessagesConverter& messages_converter,
             ocpp::messages::IMessageDispatcher&             msg_dispatcher,
             ocpp::messages::GenericMessageSender&           msg_sender)
    : GenericMessageHandler<TriggerMessageReq, TriggerMessageConf>(TRIGGERMESSAGE_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_standard_handlers()
{
    msg_dispatcher.registerHandler(TRIGGERMESSAGE_ACTION, *dynamic_cast<GenericMessageHandler<TriggerMessageReq, TriggerMessageConf>*>(this));
}

TriggerMessageManager21::~TriggerMessageManager21()
{
}

void TriggerMessageManager21::registerHandler(ocpp::types::ocpp21::MessageTriggerEnumType message, ITriggerMessageHandler& handler)
{
    m_standard_handlers[message] = &handler;
}

bool TriggerMessageManager21::handleMessage(const TriggerMessageReq& request,
                               TriggerMessageConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    (void)error_code;
    (void)error_message;
    (void)m_events_handler;
    (void)m_msg_sender;

    std::string trigger_message = MessageTriggerEnumTypeHelper.toString(request.requestedMessage);
    LOG_INFO << "Trigger message requested : " << trigger_message
             << " - EVSE = " << (request.evse.isSet() ? std::to_string(request.evse.value().id) : "not set") << " - connectorId = "
             << ((request.evse.isSet() && request.evse.value().connectorId.isSet())
                     ? std::to_string(request.evse.value().connectorId.value())
                     : "not set");

    auto it = m_standard_handlers.find(request.requestedMessage);
    if (it == m_standard_handlers.end())
    {
        response.status = TriggerMessageStatusEnumType::NotImplemented;
        LOG_WARNING << "Trigger message not implemented : " << trigger_message;
    }
    else if (it->second->onTriggerMessage(request.requestedMessage, request.evse))
    {
        response.status = TriggerMessageStatusEnumType::Accepted;
        LOG_INFO << "Trigger message accepted : " << trigger_message;
    }
    else
    {
        response.status = TriggerMessageStatusEnumType::Rejected;
        LOG_WARNING << "Trigger message rejected : " << trigger_message;
    }

    return true;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
