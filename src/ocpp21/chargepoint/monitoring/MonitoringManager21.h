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

#ifndef OPENOCPP_OCPP21_MONITORINGMANAGER21_H
#define OPENOCPP_OCPP21_MONITORINGMANAGER21_H

#include "AdjustPeriodicEventStream21.h"
#include "ClearVariableMonitoring21.h"
#include "ClosePeriodicEventStream21.h"
#include "GenericMessageHandler.h"
#include "GetMonitoringReport21.h"
#include "GetPeriodicEventStream21.h"
#include "OpenPeriodicEventStream21.h"
#include "SetMonitoringBase21.h"
#include "SetMonitoringLevel21.h"
#include "SetVariableMonitoring21.h"

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
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 monitoring requests for the charge point */
class MonitoringManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::AdjustPeriodicEventStreamReq,
                                                   ocpp::messages::ocpp21::AdjustPeriodicEventStreamConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ClearVariableMonitoringReq,
                                                   ocpp::messages::ocpp21::ClearVariableMonitoringConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ClosePeriodicEventStreamReq,
                                                   ocpp::messages::ocpp21::ClosePeriodicEventStreamConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetMonitoringReportReq,
                                                   ocpp::messages::ocpp21::GetMonitoringReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetPeriodicEventStreamReq,
                                                   ocpp::messages::ocpp21::GetPeriodicEventStreamConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::OpenPeriodicEventStreamReq,
                                                   ocpp::messages::ocpp21::OpenPeriodicEventStreamConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetMonitoringBaseReq,
                                                   ocpp::messages::ocpp21::SetMonitoringBaseConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetMonitoringLevelReq,
                                                   ocpp::messages::ocpp21::SetMonitoringLevelConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetVariableMonitoringReq,
                                                   ocpp::messages::ocpp21::SetVariableMonitoringConf>
{
  public:
    MonitoringManager21(IChargePointEventsHandler21&                    events_handler,
                        const ocpp::messages::GenericMessagesConverter& messages_converter,
                        ocpp::messages::IMessageDispatcher&             msg_dispatcher);
    virtual ~MonitoringManager21();

    bool handleMessage(const ocpp::messages::ocpp21::AdjustPeriodicEventStreamReq& request,
                       ocpp::messages::ocpp21::AdjustPeriodicEventStreamConf&      response,
                       std::string&                                                error_code,
                       std::string&                                                error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::ClearVariableMonitoringReq& request,
                       ocpp::messages::ocpp21::ClearVariableMonitoringConf&      response,
                       std::string&                                              error_code,
                       std::string&                                              error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::ClosePeriodicEventStreamReq& request,
                       ocpp::messages::ocpp21::ClosePeriodicEventStreamConf&      response,
                       std::string&                                               error_code,
                       std::string&                                               error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetMonitoringReportReq& request,
                       ocpp::messages::ocpp21::GetMonitoringReportConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetPeriodicEventStreamReq& request,
                       ocpp::messages::ocpp21::GetPeriodicEventStreamConf&      response,
                       std::string&                                             error_code,
                       std::string&                                             error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::OpenPeriodicEventStreamReq& request,
                       ocpp::messages::ocpp21::OpenPeriodicEventStreamConf&      response,
                       std::string&                                              error_code,
                       std::string&                                              error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetMonitoringBaseReq& request,
                       ocpp::messages::ocpp21::SetMonitoringBaseConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetMonitoringLevelReq& request,
                       ocpp::messages::ocpp21::SetMonitoringLevelConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetVariableMonitoringReq& request,
                       ocpp::messages::ocpp21::SetVariableMonitoringConf&      response,
                       std::string&                                            error_code,
                       std::string&                                            error_message) override;

  private:
    IChargePointEventsHandler21& m_events_handler;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_MONITORINGMANAGER21_H
