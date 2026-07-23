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

#include "MonitoringManager21.h"

#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

MonitoringManager21::MonitoringManager21(IChargePointEventsHandler21&                    events_handler,
                                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                                         ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<AdjustPeriodicEventStreamReq, AdjustPeriodicEventStreamConf>(ADJUSTPERIODICEVENTSTREAM_ACTION,
                                                                                          messages_converter),
      GenericMessageHandler<ClearVariableMonitoringReq, ClearVariableMonitoringConf>(CLEARVARIABLEMONITORING_ACTION, messages_converter),
      GenericMessageHandler<ClosePeriodicEventStreamReq, ClosePeriodicEventStreamConf>(CLOSEPERIODICEVENTSTREAM_ACTION, messages_converter),
      GenericMessageHandler<GetMonitoringReportReq, GetMonitoringReportConf>(GETMONITORINGREPORT_ACTION, messages_converter),
      GenericMessageHandler<GetPeriodicEventStreamReq, GetPeriodicEventStreamConf>(GETPERIODICEVENTSTREAM_ACTION, messages_converter),
      GenericMessageHandler<OpenPeriodicEventStreamReq, OpenPeriodicEventStreamConf>(OPENPERIODICEVENTSTREAM_ACTION, messages_converter),
      GenericMessageHandler<SetMonitoringBaseReq, SetMonitoringBaseConf>(SETMONITORINGBASE_ACTION, messages_converter),
      GenericMessageHandler<SetMonitoringLevelReq, SetMonitoringLevelConf>(SETMONITORINGLEVEL_ACTION, messages_converter),
      GenericMessageHandler<SetVariableMonitoringReq, SetVariableMonitoringConf>(SETVARIABLEMONITORING_ACTION, messages_converter),
      m_events_handler(events_handler)
{
    msg_dispatcher.registerHandler(
        ADJUSTPERIODICEVENTSTREAM_ACTION,
        *dynamic_cast<GenericMessageHandler<AdjustPeriodicEventStreamReq, AdjustPeriodicEventStreamConf>*>(this));
    msg_dispatcher.registerHandler(
        CLEARVARIABLEMONITORING_ACTION,
        *dynamic_cast<GenericMessageHandler<ClearVariableMonitoringReq, ClearVariableMonitoringConf>*>(this));
    msg_dispatcher.registerHandler(
        CLOSEPERIODICEVENTSTREAM_ACTION,
        *dynamic_cast<GenericMessageHandler<ClosePeriodicEventStreamReq, ClosePeriodicEventStreamConf>*>(this));
    msg_dispatcher.registerHandler(GETMONITORINGREPORT_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetMonitoringReportReq, GetMonitoringReportConf>*>(this));
    msg_dispatcher.registerHandler(
        GETPERIODICEVENTSTREAM_ACTION,
        *dynamic_cast<GenericMessageHandler<GetPeriodicEventStreamReq, GetPeriodicEventStreamConf>*>(this));
    msg_dispatcher.registerHandler(
        OPENPERIODICEVENTSTREAM_ACTION,
        *dynamic_cast<GenericMessageHandler<OpenPeriodicEventStreamReq, OpenPeriodicEventStreamConf>*>(this));
    msg_dispatcher.registerHandler(SETMONITORINGBASE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetMonitoringBaseReq, SetMonitoringBaseConf>*>(this));
    msg_dispatcher.registerHandler(SETMONITORINGLEVEL_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetMonitoringLevelReq, SetMonitoringLevelConf>*>(this));
    msg_dispatcher.registerHandler(
        SETVARIABLEMONITORING_ACTION,
        *dynamic_cast<GenericMessageHandler<SetVariableMonitoringReq, SetVariableMonitoringConf>*>(this));
}

MonitoringManager21::~MonitoringManager21() { }

bool MonitoringManager21::handleMessage(const AdjustPeriodicEventStreamReq& request,
                                        AdjustPeriodicEventStreamConf&      response,
                                        std::string&                        error_code,
                                        std::string&                        error_message)
{
    LOG_INFO << "AdjustPeriodicEventStream request received";
    return m_events_handler.onAdjustPeriodicEventStream(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const ClearVariableMonitoringReq& request,
                                        ClearVariableMonitoringConf&      response,
                                        std::string&                      error_code,
                                        std::string&                      error_message)
{
    LOG_INFO << "ClearVariableMonitoring request received";
    return m_events_handler.onClearVariableMonitoring(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const ClosePeriodicEventStreamReq& request,
                                        ClosePeriodicEventStreamConf&      response,
                                        std::string&                       error_code,
                                        std::string&                       error_message)
{
    LOG_INFO << "ClosePeriodicEventStream request received";
    return m_events_handler.onClosePeriodicEventStream(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const GetMonitoringReportReq& request,
                                        GetMonitoringReportConf&      response,
                                        std::string&                  error_code,
                                        std::string&                  error_message)
{
    LOG_INFO << "GetMonitoringReport request received";
    return m_events_handler.onGetMonitoringReport(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const GetPeriodicEventStreamReq& request,
                                        GetPeriodicEventStreamConf&      response,
                                        std::string&                     error_code,
                                        std::string&                     error_message)
{
    LOG_INFO << "GetPeriodicEventStream request received";
    return m_events_handler.onGetPeriodicEventStream(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const OpenPeriodicEventStreamReq& request,
                                        OpenPeriodicEventStreamConf&      response,
                                        std::string&                      error_code,
                                        std::string&                      error_message)
{
    LOG_INFO << "OpenPeriodicEventStream request received";
    return m_events_handler.onOpenPeriodicEventStream(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const SetMonitoringBaseReq& request,
                                        SetMonitoringBaseConf&      response,
                                        std::string&                error_code,
                                        std::string&                error_message)
{
    LOG_INFO << "SetMonitoringBase request received";
    return m_events_handler.onSetMonitoringBase(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const SetMonitoringLevelReq& request,
                                        SetMonitoringLevelConf&      response,
                                        std::string&                 error_code,
                                        std::string&                 error_message)
{
    LOG_INFO << "SetMonitoringLevel request received";
    return m_events_handler.onSetMonitoringLevel(request, response, error_code, error_message);
}

bool MonitoringManager21::handleMessage(const SetVariableMonitoringReq& request,
                                        SetVariableMonitoringConf&      response,
                                        std::string&                    error_code,
                                        std::string&                    error_message)
{
    LOG_INFO << "SetVariableMonitoring request received";
    return m_events_handler.onSetVariableMonitoring(request, response, error_code, error_message);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
