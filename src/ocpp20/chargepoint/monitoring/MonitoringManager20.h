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

#ifndef OPENOCPP_OCPP20_MONITORINGMANAGER20_H
#define OPENOCPP_OCPP20_MONITORINGMANAGER20_H

#include "ClearVariableMonitoring20.h"
#include "GenericMessageHandler.h"
#include "GetMonitoringReport20.h"
#include "SetMonitoringBase20.h"
#include "SetMonitoringLevel20.h"
#include "SetVariableMonitoring20.h"

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

/** @brief Handle OCPP 2.0.1 monitoring requests for the charge point */
class MonitoringManager20
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ClearVariableMonitoringReq,
                                                   ocpp::messages::ocpp20::ClearVariableMonitoringConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetMonitoringReportReq,
                                                   ocpp::messages::ocpp20::GetMonitoringReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetMonitoringBaseReq,
                                                   ocpp::messages::ocpp20::SetMonitoringBaseConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetMonitoringLevelReq,
                                                   ocpp::messages::ocpp20::SetMonitoringLevelConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetVariableMonitoringReq,
                                                   ocpp::messages::ocpp20::SetVariableMonitoringConf>
{
  public:
    /** @brief Constructor */
    MonitoringManager20(IChargePointEventsHandler20&                    events_handler,
                        const ocpp::messages::GenericMessagesConverter& messages_converter,
                        ocpp::messages::IMessageDispatcher&             msg_dispatcher);

    /** @brief Destructor */
    virtual ~MonitoringManager20();

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::ClearVariableMonitoringReq& request,
                       ocpp::messages::ocpp20::ClearVariableMonitoringConf&      response,
                       std::string&                                              error_code,
                       std::string&                                              error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetMonitoringReportReq& request,
                       ocpp::messages::ocpp20::GetMonitoringReportConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetMonitoringBaseReq& request,
                       ocpp::messages::ocpp20::SetMonitoringBaseConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetMonitoringLevelReq& request,
                       ocpp::messages::ocpp20::SetMonitoringLevelConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetVariableMonitoringReq& request,
                       ocpp::messages::ocpp20::SetVariableMonitoringConf&      response,
                       std::string&                                            error_code,
                       std::string&                                            error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_MONITORINGMANAGER20_H
