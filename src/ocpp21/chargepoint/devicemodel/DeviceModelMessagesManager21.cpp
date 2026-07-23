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

#include "DeviceModelMessagesManager21.h"

#include "GenericMessageSender.h"
#include "IDeviceModel21.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "NotifyReport21.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types;
using namespace ocpp::chargepoint::ocpp21;
using namespace ocpp::types::ocpp21;

namespace
{

static constexpr std::size_t MAX_NOTIFY_REPORT_ITEMS = 100u;

bool isAllowedInstanceChar(char c)
{
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '*') ||
            (c == '-') || (c == '_') || (c == '=') || (c == ':') || (c == '+') || (c == '|') || (c == '@') ||
            (c == '.'));
}

std::string sanitizeInstance(const std::string& instance)
{
    std::string sanitized;
    sanitized.reserve(instance.size());
    for (char c : instance)
    {
        sanitized.push_back(isAllowedInstanceChar(c) ? c : '_');
    }
    return sanitized;
}

bool sameComponent(const ComponentType& left, const DeviceModelComponent21& right)
{
    if (left.name.str() != right.name)
    {
        return false;
    }
    if (left.instance.isSet() && (!right.instance.isSet() || (left.instance.value().str() != right.instance.value())))
    {
        return false;
    }
    if (left.evse.isSet())
    {
        if (!right.evse.isSet() || (left.evse.value().id != static_cast<int>(right.evse.value())))
        {
            return false;
        }
        if (left.evse.value().connectorId.isSet() &&
            (!right.connector.isSet() || (left.evse.value().connectorId.value() != static_cast<int>(right.connector.value()))))
        {
            return false;
        }
    }
    return true;
}

bool sameVariable(const VariableType& left, const DeviceModelVariable21& right)
{
    if (left.name.str() != right.name)
    {
        return false;
    }
    if (left.instance.isSet() && (!right.instance.isSet() || (left.instance.value().str() != right.instance.value())))
    {
        return false;
    }
    return true;
}

void fillComponent(ComponentType& type, const DeviceModelComponent21& component)
{
    type.name.assign(component.name);
    if (component.instance.isSet())
    {
        type.instance.value().assign(sanitizeInstance(component.instance.value()));
    }
    if (component.evse.isSet())
    {
        type.evse.value().id = component.evse.value();
        if (component.connector.isSet())
        {
            type.evse.value().connectorId.value() = component.connector.value();
        }
    }
}

void fillVariable(VariableType& type, const DeviceModelVariable21& var)
{
    type.name.assign(var.name);
    if (var.instance.isSet())
    {
        type.instance.value().assign(sanitizeInstance(var.instance.value()));
    }
}

} // namespace

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

DeviceModelMessagesManager21::DeviceModelMessagesManager21(IDeviceModel21&                                 device_model,
                                                           const GenericMessagesConverter&                  messages_converter,
                                                           IMessageDispatcher&                              msg_dispatcher,
                                                           GenericMessageSender&                            msg_sender)
    : GenericMessageHandler<GetBaseReportReq, GetBaseReportConf>(GETBASEREPORT_ACTION, messages_converter),
      GenericMessageHandler<GetReportReq, GetReportConf>(GETREPORT_ACTION, messages_converter),
      GenericMessageHandler<GetVariablesReq, GetVariablesConf>(GETVARIABLES_ACTION, messages_converter),
      GenericMessageHandler<SetVariablesReq, SetVariablesConf>(SETVARIABLES_ACTION, messages_converter),
      m_device_model(device_model),
      m_msg_sender(msg_sender)
{
    msg_dispatcher.registerHandler(GETBASEREPORT_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetBaseReportReq, GetBaseReportConf>*>(this));
    msg_dispatcher.registerHandler(GETREPORT_ACTION, *dynamic_cast<GenericMessageHandler<GetReportReq, GetReportConf>*>(this));
    msg_dispatcher.registerHandler(GETVARIABLES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetVariablesReq, GetVariablesConf>*>(this));
    msg_dispatcher.registerHandler(SETVARIABLES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetVariablesReq, SetVariablesConf>*>(this));
}

DeviceModelMessagesManager21::~DeviceModelMessagesManager21() { }

bool DeviceModelMessagesManager21::handleMessage(const GetBaseReportReq& request,
                                                 GetBaseReportConf&      response,
                                                 std::string&            error_code,
                                                 std::string&            error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "GetBaseReport request received";
    std::vector<ReportDataType> report_data;
    collectReportData(report_data);
    response.status = report_data.empty() ? GenericDeviceModelStatusEnumType::EmptyResultSet
                                          : GenericDeviceModelStatusEnumType::Accepted;
    if (response.status == GenericDeviceModelStatusEnumType::Accepted)
    {
        (void)sendNotifyReport(request.requestId, report_data);
    }
    return true;
}

bool DeviceModelMessagesManager21::handleMessage(const GetReportReq& request,
                                                 GetReportConf&      response,
                                                 std::string&        error_code,
                                                 std::string&        error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "GetReport request received";
    std::vector<ReportDataType> report_data;
    if (request.componentVariable.empty())
    {
        collectReportData(report_data);
    }
    else
    {
        for (const auto& component_var : request.componentVariable)
        {
            collectReportData(report_data, &component_var);
        }
    }
    response.status = report_data.empty() ? GenericDeviceModelStatusEnumType::EmptyResultSet
                                          : GenericDeviceModelStatusEnumType::Accepted;
    if (response.status == GenericDeviceModelStatusEnumType::Accepted)
    {
        (void)sendNotifyReport(request.requestId, report_data);
    }
    return true;
}

bool DeviceModelMessagesManager21::handleMessage(const GetVariablesReq& request,
                                                 GetVariablesConf&      response,
                                                 std::string&           error_code,
                                                 std::string&           error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "GetVariables request received";
    response.getVariableResult.clear();
    for (const auto& var_data : request.getVariableData)
    {
        response.getVariableResult.push_back(m_device_model.getVariable(var_data));
    }
    return true;
}

bool DeviceModelMessagesManager21::handleMessage(const SetVariablesReq& request,
                                                 SetVariablesConf&      response,
                                                 std::string&           error_code,
                                                 std::string&           error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "SetVariables request received";
    response.setVariableResult.clear();
    for (const auto& var_data : request.setVariableData)
    {
        response.setVariableResult.push_back(m_device_model.setVariable(var_data));
    }
    return true;
}

void DeviceModelMessagesManager21::collectReportData(std::vector<ReportDataType>& report_data,
                                                     const ComponentVariableType* filter) const
{
    const auto& device_model = m_device_model.getModel();
    for (const auto& [_, components] : device_model.components)
    {
        for (const auto& component : components)
        {
            if (filter && !sameComponent(filter->component, component))
            {
                continue;
            }
            for (const auto& [__, vars] : component.variables)
            {
                for (const auto& [___, var] : vars)
                {
                    if (filter && filter->variable.isSet() && !sameVariable(filter->variable.value(), var))
                    {
                        continue;
                    }

                    ReportDataType data;
                    fillComponent(data.component, component);
                    fillVariable(data.variable, var);
                    data.variableAttribute.push_back(var.attributes);
                    data.variableCharacteristics.value() = var.characteristics;
                    report_data.push_back(std::move(data));
                }
            }
        }
    }
}

bool DeviceModelMessagesManager21::sendNotifyReport(int request_id, const std::vector<ReportDataType>& report_data)
{
    bool   ret    = true;
    int    seq_no = 0;
    size_t offset = 0u;
    while (offset < report_data.size())
    {
        const size_t remaining = report_data.size() - offset;
        const size_t count     = (remaining > MAX_NOTIFY_REPORT_ITEMS) ? MAX_NOTIFY_REPORT_ITEMS : remaining;
        const size_t end       = offset + count;

        NotifyReportReq  notify_req;
        NotifyReportConf notify_conf;
        notify_req.requestId   = request_id;
        notify_req.generatedAt = DateTime::now();
        notify_req.reportData.assign(report_data.begin() + static_cast<std::ptrdiff_t>(offset),
                                     report_data.begin() + static_cast<std::ptrdiff_t>(end));
        notify_req.tbc.value() = (end < report_data.size());
        notify_req.seqNo       = seq_no;

        std::string      error;
        std::string      message;
        const CallResult call_result = m_msg_sender.call(NOTIFYREPORT_ACTION, notify_req, notify_conf, error, message);
        if (call_result != CallResult::Ok)
        {
            LOG_ERROR << "NotifyReport => " << CallResultHelper.toString(call_result) << " - seqNo = " << seq_no
                      << " - items = " << count << " - error = " << error << " - message = " << message;
            ret = false;
            break;
        }

        offset = end;
        ++seq_no;
    }
    return ret;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
