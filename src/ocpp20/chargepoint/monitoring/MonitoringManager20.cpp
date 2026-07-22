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

#include "MonitoringManager20.h"

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
MonitoringManager20::MonitoringManager20(IChargePointEventsHandler20&                    events_handler,
                                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                                         ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<ClearVariableMonitoringReq, ClearVariableMonitoringConf>(CLEARVARIABLEMONITORING_ACTION, messages_converter),
      GenericMessageHandler<GetMonitoringReportReq, GetMonitoringReportConf>(GETMONITORINGREPORT_ACTION, messages_converter),
      GenericMessageHandler<SetMonitoringBaseReq, SetMonitoringBaseConf>(SETMONITORINGBASE_ACTION, messages_converter),
      GenericMessageHandler<SetMonitoringLevelReq, SetMonitoringLevelConf>(SETMONITORINGLEVEL_ACTION, messages_converter),
      GenericMessageHandler<SetVariableMonitoringReq, SetVariableMonitoringConf>(SETVARIABLEMONITORING_ACTION, messages_converter),
      m_events_handler(events_handler)
{
    msg_dispatcher.registerHandler(
        CLEARVARIABLEMONITORING_ACTION,
        *dynamic_cast<GenericMessageHandler<ClearVariableMonitoringReq, ClearVariableMonitoringConf>*>(this));
    msg_dispatcher.registerHandler(GETMONITORINGREPORT_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetMonitoringReportReq, GetMonitoringReportConf>*>(this));
    msg_dispatcher.registerHandler(SETMONITORINGBASE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetMonitoringBaseReq, SetMonitoringBaseConf>*>(this));
    msg_dispatcher.registerHandler(SETMONITORINGLEVEL_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetMonitoringLevelReq, SetMonitoringLevelConf>*>(this));
    msg_dispatcher.registerHandler(
        SETVARIABLEMONITORING_ACTION,
        *dynamic_cast<GenericMessageHandler<SetVariableMonitoringReq, SetVariableMonitoringConf>*>(this));
}

/** @brief Destructor */
MonitoringManager20::~MonitoringManager20() { }

/** @brief Handle ClearVariableMonitoring */
bool MonitoringManager20::handleMessage(const ClearVariableMonitoringReq& request,
                                        ClearVariableMonitoringConf&      response,
                                        std::string&                      error_code,
                                        std::string&                      error_message)
{
    LOG_INFO << "ClearVariableMonitoring request received";
    return m_events_handler.onClearVariableMonitoring(request, response, error_code, error_message);
}

/** @brief Handle GetMonitoringReport */
bool MonitoringManager20::handleMessage(const GetMonitoringReportReq& request,
                                        GetMonitoringReportConf&      response,
                                        std::string&                  error_code,
                                        std::string&                  error_message)
{
    LOG_INFO << "GetMonitoringReport request received";
    return m_events_handler.onGetMonitoringReport(request, response, error_code, error_message);
}

/** @brief Handle SetMonitoringBase */
bool MonitoringManager20::handleMessage(const SetMonitoringBaseReq& request,
                                        SetMonitoringBaseConf&      response,
                                        std::string&                error_code,
                                        std::string&                error_message)
{
    LOG_INFO << "SetMonitoringBase request received";
    return m_events_handler.onSetMonitoringBase(request, response, error_code, error_message);
}

/** @brief Handle SetMonitoringLevel */
bool MonitoringManager20::handleMessage(const SetMonitoringLevelReq& request,
                                        SetMonitoringLevelConf&      response,
                                        std::string&                 error_code,
                                        std::string&                 error_message)
{
    LOG_INFO << "SetMonitoringLevel request received";
    return m_events_handler.onSetMonitoringLevel(request, response, error_code, error_message);
}

/** @brief Handle SetVariableMonitoring */
bool MonitoringManager20::handleMessage(const SetVariableMonitoringReq& request,
                                        SetVariableMonitoringConf&      response,
                                        std::string&                    error_code,
                                        std::string&                    error_message)
{
    LOG_INFO << "SetVariableMonitoring request received";
    return m_events_handler.onSetVariableMonitoring(request, response, error_code, error_message);
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
