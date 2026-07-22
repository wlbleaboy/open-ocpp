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

#ifndef OPENOCPP_OCPP20_DEVICEMODELMESSAGESMANAGER20_H
#define OPENOCPP_OCPP20_DEVICEMODELMESSAGESMANAGER20_H

#include "GenericMessageHandler.h"
#include "GetBaseReport20.h"
#include "GetReport20.h"
#include "GetVariables20.h"
#include "SetVariables20.h"

#include <string>

namespace ocpp
{
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

/** @brief Handle OCPP 2.0.1 device model requests for the charge point */
class DeviceModelMessagesManager20
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetBaseReportReq,
                                                   ocpp::messages::ocpp20::GetBaseReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetReportReq, ocpp::messages::ocpp20::GetReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetVariablesReq,
                                                   ocpp::messages::ocpp20::GetVariablesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetVariablesReq,
                                                   ocpp::messages::ocpp20::SetVariablesConf>
{
  public:
    /** @brief Constructor */
    DeviceModelMessagesManager20(IChargePointEventsHandler20&                    events_handler,
                                 const ocpp::messages::GenericMessagesConverter& messages_converter,
                                 ocpp::messages::IMessageDispatcher&             msg_dispatcher);

    /** @brief Destructor */
    virtual ~DeviceModelMessagesManager20();

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::GetBaseReportReq& request,
                       ocpp::messages::ocpp20::GetBaseReportConf&      response,
                       std::string&                                    error_code,
                       std::string&                                    error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetReportReq& request,
                       ocpp::messages::ocpp20::GetReportConf&      response,
                       std::string&                                error_code,
                       std::string&                                error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetVariablesReq& request,
                       ocpp::messages::ocpp20::GetVariablesConf&      response,
                       std::string&                                   error_code,
                       std::string&                                   error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetVariablesReq& request,
                       ocpp::messages::ocpp20::SetVariablesConf&      response,
                       std::string&                                   error_code,
                       std::string&                                   error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_DEVICEMODELMESSAGESMANAGER20_H
