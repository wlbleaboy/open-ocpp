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

#include "DeviceModelMessagesManager20.h"

#include "IChargePointEventsHandler20.h"
#include "IMessageDispatcher.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
DeviceModelMessagesManager20::DeviceModelMessagesManager20(IChargePointEventsHandler20&                    events_handler,
                                                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                                                           ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<GetBaseReportReq, GetBaseReportConf>(GETBASEREPORT_ACTION, messages_converter),
      GenericMessageHandler<GetReportReq, GetReportConf>(GETREPORT_ACTION, messages_converter),
      GenericMessageHandler<GetVariablesReq, GetVariablesConf>(GETVARIABLES_ACTION, messages_converter),
      GenericMessageHandler<SetVariablesReq, SetVariablesConf>(SETVARIABLES_ACTION, messages_converter),
      m_events_handler(events_handler)
{
    msg_dispatcher.registerHandler(GETBASEREPORT_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetBaseReportReq, GetBaseReportConf>*>(this));
    msg_dispatcher.registerHandler(GETREPORT_ACTION, *dynamic_cast<GenericMessageHandler<GetReportReq, GetReportConf>*>(this));
    msg_dispatcher.registerHandler(GETVARIABLES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetVariablesReq, GetVariablesConf>*>(this));
    msg_dispatcher.registerHandler(SETVARIABLES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetVariablesReq, SetVariablesConf>*>(this));
}

/** @brief Destructor */
DeviceModelMessagesManager20::~DeviceModelMessagesManager20() { }

/** @brief Handle GetBaseReport */
bool DeviceModelMessagesManager20::handleMessage(const GetBaseReportReq& request,
                                                 GetBaseReportConf&      response,
                                                 std::string&            error_code,
                                                 std::string&            error_message)
{
    LOG_INFO << "GetBaseReport request received";
    return m_events_handler.onGetBaseReport(request, response, error_code, error_message);
}

/** @brief Handle GetReport */
bool DeviceModelMessagesManager20::handleMessage(const GetReportReq& request,
                                                 GetReportConf&      response,
                                                 std::string&        error_code,
                                                 std::string&        error_message)
{
    LOG_INFO << "GetReport request received";
    return m_events_handler.onGetReport(request, response, error_code, error_message);
}

/** @brief Handle GetVariables */
bool DeviceModelMessagesManager20::handleMessage(const GetVariablesReq& request,
                                                 GetVariablesConf&      response,
                                                 std::string&           error_code,
                                                 std::string&           error_message)
{
    LOG_INFO << "GetVariables request received";
    return m_events_handler.onGetVariables(request, response, error_code, error_message);
}

/** @brief Handle SetVariables */
bool DeviceModelMessagesManager20::handleMessage(const SetVariablesReq& request,
                                                 SetVariablesConf&      response,
                                                 std::string&           error_code,
                                                 std::string&           error_message)
{
    LOG_INFO << "SetVariables request received";
    return m_events_handler.onSetVariables(request, response, error_code, error_message);
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
