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

#ifndef OPENOCPP_OCPP21_TRIGGERMESSAGEMANAGER21_H
#define OPENOCPP_OCPP21_TRIGGERMESSAGEMANAGER21_H

#include "GenericMessageHandler.h"
#include "ITriggerMessageManager21.h"
#include "TriggerMessage21.h"

#include <map>

namespace ocpp
{
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

class IChargePointEventsHandler21;

/** @brief Manage OCPP 2.1 TriggerMessage requests */
class TriggerMessageManager21
    : public ITriggerMessageManager21,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::TriggerMessageReq, ocpp::messages::ocpp21::TriggerMessageConf>
{
  public:
    TriggerMessageManager21(IChargePointEventsHandler21&                         events_handler,
          const ocpp::messages::GenericMessagesConverter&      messages_converter,
          ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
          ocpp::messages::GenericMessageSender&                msg_sender);
    virtual ~TriggerMessageManager21();

    void registerHandler(ocpp::types::ocpp21::MessageTriggerEnumType message, ITriggerMessageHandler& handler) override;

    bool handleMessage(const ocpp::messages::ocpp21::TriggerMessageReq& request,
                       ocpp::messages::ocpp21::TriggerMessageConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;

  private:
    IChargePointEventsHandler21& m_events_handler;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    std::map<ocpp::types::ocpp21::MessageTriggerEnumType, ITriggerMessageHandler*> m_standard_handlers;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_TRIGGERMESSAGEMANAGER21_H
