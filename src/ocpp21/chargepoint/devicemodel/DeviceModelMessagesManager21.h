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

#ifndef OPENOCPP_OCPP21_DEVICEMODELMESSAGESMANAGER21_H
#define OPENOCPP_OCPP21_DEVICEMODELMESSAGESMANAGER21_H

#include "GenericMessageHandler.h"
#include "GetBaseReport21.h"
#include "GetReport21.h"
#include "GetVariables21.h"
#include "ReportDataType21.h"
#include "SetVariables21.h"

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
class IMessageDispatcher;
class GenericMessagesConverter;
} // namespace messages
namespace chargepoint
{
namespace ocpp21
{

class IDeviceModel21;

/** @brief Manager for OCPP 2.1 device model messages */
class DeviceModelMessagesManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetBaseReportReq,
                                                   ocpp::messages::ocpp21::GetBaseReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetReportReq,
                                                   ocpp::messages::ocpp21::GetReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetVariablesReq,
                                                   ocpp::messages::ocpp21::GetVariablesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetVariablesReq,
                                                   ocpp::messages::ocpp21::SetVariablesConf>
{
  public:
    DeviceModelMessagesManager21(IDeviceModel21&                                      device_model,
                                 const ocpp::messages::GenericMessagesConverter&      messages_converter,
                                 ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
                                 ocpp::messages::GenericMessageSender&                msg_sender);
    ~DeviceModelMessagesManager21() override;

    bool handleMessage(const ocpp::messages::ocpp21::GetBaseReportReq& request,
                       ocpp::messages::ocpp21::GetBaseReportConf&      response,
                       std::string&                                    error_code,
                       std::string&                                    error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetReportReq& request,
                       ocpp::messages::ocpp21::GetReportConf&      response,
                       std::string&                                error_code,
                       std::string&                                error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetVariablesReq& request,
                       ocpp::messages::ocpp21::GetVariablesConf&      response,
                       std::string&                                  error_code,
                       std::string&                                  error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetVariablesReq& request,
                       ocpp::messages::ocpp21::SetVariablesConf&      response,
                       std::string&                                  error_code,
                       std::string&                                  error_message) override;

  private:
    IDeviceModel21&                       m_device_model;
    ocpp::messages::GenericMessageSender& m_msg_sender;

    void collectReportData(std::vector<ocpp::types::ocpp21::ReportDataType>& report_data,
                           const ocpp::types::ocpp21::ComponentVariableType* filter = nullptr) const;
    bool sendNotifyReport(int request_id, const std::vector<ocpp::types::ocpp21::ReportDataType>& report_data);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_DEVICEMODELMESSAGESMANAGER21_H
