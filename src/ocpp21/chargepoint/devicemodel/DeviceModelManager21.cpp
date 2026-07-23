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

#include "DeviceModelManager21.h"
#include "StringHelpers.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

using namespace ocpp::types;
using namespace ocpp::types::ocpp21;

namespace
{

std::string optionalString(const Optional<std::string>& value)
{
    return value.isSet() ? value.value() : std::string();
}

std::string optionalString(const Optional<CiStringType<50u>>& value)
{
    return value.isSet() ? value.value().str() : std::string();
}

void setComponentType(ComponentType& type, const ocpp::chargepoint::ocpp21::DeviceModelComponent21& component)
{
    type.name.assign(component.name);
    if (component.instance.isSet())
    {
        type.instance.value().assign(component.instance.value());
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

void setVariableType(VariableType& type, const ocpp::chargepoint::ocpp21::DeviceModelVariable21& var)
{
    type.name.assign(var.name);
    if (var.instance.isSet())
    {
        type.instance.value().assign(var.instance.value());
    }
}

ocpp::chargepoint::ocpp21::DeviceModelVariable21 makeStringVariable(const std::string& name, const std::string& value)
{
    ocpp::chargepoint::ocpp21::DeviceModelVariable21 var;
    var.name = name;
    var.attributes.type.value()       = AttributeEnumType::Actual;
    var.attributes.value.value().assign(value);
    var.attributes.mutability.value() = MutabilityEnumType::ReadOnly;
    var.attributes.persistent.value() = true;
    var.attributes.constant.value()   = true;
    var.characteristics.dataType           = DataEnumType::string;
    var.characteristics.supportsMonitoring = false;
    return var;
}

} // namespace

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

DeviceModelManager21::DeviceModelManager21(const ocpp::config::IChargePointConfig21& stack_config)
    : m_stack_config(stack_config), m_validator(), m_last_error(), m_device_model(), m_listener(nullptr)
{
    loadDefaultModel();
}

DeviceModelManager21::~DeviceModelManager21() { }

bool DeviceModelManager21::init()
{
    std::filesystem::path device_model_schema_path = m_stack_config.jsonSchemasPath();
    device_model_schema_path.append("device_model_schema.json");
    const bool ret = m_validator.init(device_model_schema_path);
    if (!ret)
    {
        m_last_error = m_validator.lastError();
    }
    return ret;
}

bool DeviceModelManager21::load(const std::string& device_model_file_path)
{
    bool ret = false;

    std::ifstream device_model_file;
    device_model_file.open(device_model_file_path);
    if (device_model_file.is_open())
    {
        rapidjson::IStreamWrapper device_model_file_wrapper(device_model_file);
        rapidjson::Document       device_model_doc;
        device_model_doc.ParseStream(device_model_file_wrapper);
        rapidjson::ParseErrorCode error = device_model_doc.GetParseError();
        if (error == rapidjson::ParseErrorCode::kParseErrorNone)
        {
            ret = m_validator.isValid(device_model_doc);
            if (ret)
            {
                loadDeviceModel(device_model_doc);
            }
            else
            {
                m_last_error = m_validator.lastError();
            }
        }
        else
        {
            m_last_error = rapidjson::GetParseError_En(error);
        }
    }
    else
    {
        m_last_error = "Unable to open device model file : " + device_model_file_path;
    }

    return ret;
}

bool DeviceModelManager21::save(const std::string& device_model_file_path)
{
    bool ret = false;

    std::ofstream device_model_file;
    device_model_file.open(device_model_file_path);
    if (device_model_file.is_open())
    {
        rapidjson::Document device_model_doc(rapidjson::kObjectType);
        saveDeviceModel(device_model_doc);

        rapidjson::OStreamWrapper                    device_model_file_wrapper(device_model_file);
        rapidjson::Writer<rapidjson::OStreamWrapper> device_model_file_writer(device_model_file_wrapper);
        ret = device_model_doc.Accept(device_model_file_writer);
        if (!ret)
        {
            m_last_error = "Unable to write to device model file : " + device_model_file_path;
        }
    }
    else
    {
        m_last_error = "Unable to open device model file : " + device_model_file_path;
    }

    return ret;
}

void DeviceModelManager21::loadDefaultModel()
{
    m_device_model.components.clear();

    DeviceModelComponent21 charging_station;
    charging_station.name = "ChargingStation";
    charging_station.variables["Model"][""]        = makeStringVariable("Model", m_stack_config.chargePointModel());
    charging_station.variables["SerialNumber"][""] = makeStringVariable("SerialNumber", m_stack_config.chargePointSerialNumber());
    charging_station.variables["VendorName"][""]   = makeStringVariable("VendorName", m_stack_config.chargePointVendor());
    charging_station.variables["FirmwareVersion"][""] = makeStringVariable("FirmwareVersion", m_stack_config.firmwareVersion());
    m_device_model.components[charging_station.name].push_back(std::move(charging_station));

    DeviceModelComponent21 device_data_ctrlr;
    device_data_ctrlr.name = "DeviceDataCtrlr";
    device_data_ctrlr.variables["ItemsPerMessage"][""] = makeStringVariable("ItemsPerMessage", "100");
    device_data_ctrlr.variables["BytesPerMessage"][""] = makeStringVariable("BytesPerMessage", "100000");
    device_data_ctrlr.variables["ConfigurationValueSize"][""] = makeStringVariable("ConfigurationValueSize", "1000");
    device_data_ctrlr.variables["ReportingValueSize"][""]     = makeStringVariable("ReportingValueSize", "2500");
    m_device_model.components[device_data_ctrlr.name].push_back(std::move(device_data_ctrlr));
}

GetVariableResultType DeviceModelManager21::getVariable(const GetVariableDataType& requested_var)
{
    GetVariableResultType result;
    result.component       = requested_var.component;
    result.variable        = requested_var.variable;
    result.attributeType   = requested_var.attributeType;
    result.attributeStatus = GetVariableStatusEnumType::Rejected;

    const DeviceModelComponent21* component = getComponent(requested_var.component);
    if (component)
    {
        bool                         not_supported_attribute_type = false;
        const DeviceModelVariable21* var =
            getVariable(*component, requested_var.attributeType, requested_var.variable, not_supported_attribute_type);
        if (var)
        {
            setComponentType(result.component, *component);
            setVariableType(result.variable, *var);
            result.attributeType.value() =
                requested_var.attributeType.isSet() ? requested_var.attributeType.value() : AttributeEnumType::Actual;
            if (var->attributes.value.isSet())
            {
                result.attributeValue.value().assign(var->attributes.value.value().str());
            }
            result.attributeStatus = GetVariableStatusEnumType::Accepted;
            if (m_listener)
            {
                m_listener->getVariable(result);
            }
        }
        else
        {
            result.attributeStatus = not_supported_attribute_type ? GetVariableStatusEnumType::NotSupportedAttributeType
                                                                  : GetVariableStatusEnumType::UnknownVariable;
        }
    }
    else
    {
        result.attributeStatus = GetVariableStatusEnumType::UnknownComponent;
    }

    return result;
}

SetVariableResultType DeviceModelManager21::setVariable(const SetVariableDataType& requested_var)
{
    return setVariable(requested_var, true);
}

SetVariableResultType DeviceModelManager21::updateVariable(const SetVariableDataType& requested_var)
{
    return setVariable(requested_var, false);
}

void DeviceModelManager21::loadDeviceModel(const rapidjson::Document& device_model_doc)
{
    m_device_model.components.clear();

    const rapidjson::Value& components_json = device_model_doc["components"];
    for (auto it_comps = components_json.Begin(); it_comps != components_json.End(); ++it_comps)
    {
        DeviceModelComponent21 component;
        const rapidjson::Value& component_json = *it_comps;

        component.name = component_json["name"].GetString();
        if (component_json.HasMember("evse"))
        {
            component.evse.value() = component_json["evse"].GetUint();
        }
        if (component_json.HasMember("connector"))
        {
            component.connector.value() = component_json["connector"].GetUint();
        }
        if (component_json.HasMember("instance"))
        {
            component.instance.value() = component_json["instance"].GetString();
        }

        const rapidjson::Value& variables_json = component_json["variables"];
        for (auto it_vars = variables_json.Begin(); it_vars != variables_json.End(); ++it_vars)
        {
            DeviceModelVariable21 var;
            const rapidjson::Value& var_json = *it_vars;

            var.name = var_json["name"].GetString();
            if (var_json.HasMember("instance"))
            {
                var.instance.value() = var_json["instance"].GetString();
            }

            const rapidjson::Value& var_attributes_json = var_json["attributes"];
            if (var_attributes_json.HasMember("type"))
            {
                var.attributes.type.value() = AttributeEnumTypeHelper.fromString(var_attributes_json["type"].GetString());
            }
            else
            {
                var.attributes.type.value() = AttributeEnumType::Actual;
            }
            if (var_attributes_json.HasMember("value"))
            {
                var.attributes.value.value().assign(var_attributes_json["value"].GetString());
            }
            if (var_attributes_json.HasMember("mutability"))
            {
                var.attributes.mutability.value() = MutabilityEnumTypeHelper.fromString(var_attributes_json["mutability"].GetString());
            }
            else
            {
                var.attributes.mutability.value() = MutabilityEnumType::ReadWrite;
            }
            var.attributes.persistent.value() =
                var_attributes_json.HasMember("persistent") ? var_attributes_json["persistent"].GetBool() : true;
            var.attributes.constant.value() =
                var_attributes_json.HasMember("constant") ? var_attributes_json["constant"].GetBool() : false;

            const rapidjson::Value& var_chars_json = var_json["characteristics"];
            var.characteristics.dataType = DataEnumTypeHelper.fromString(var_chars_json["data_type"].GetString());
            if (var_chars_json.HasMember("unit"))
            {
                var.characteristics.unit.value().assign(var_chars_json["unit"].GetString());
            }
            if (var_chars_json.HasMember("min_limit"))
            {
                var.characteristics.minLimit.value() = var_chars_json["min_limit"].GetFloat();
            }
            if (var_chars_json.HasMember("max_limit"))
            {
                var.characteristics.maxLimit.value() = var_chars_json["max_limit"].GetFloat();
            }
            if (var_chars_json.HasMember("max_elements"))
            {
                var.characteristics.maxElements.value() = var_chars_json["max_elements"].GetInt();
            }
            if (var_chars_json.HasMember("values_list"))
            {
                var.characteristics.valuesList.value().assign(var_chars_json["values_list"].GetString());
            }
            var.characteristics.supportsMonitoring =
                var_chars_json.HasMember("supports_monitoring") ? var_chars_json["supports_monitoring"].GetBool() : true;

            component.variables[var.name][optionalString(var.instance)] = std::move(var);
        }

        m_device_model.components[component.name].push_back(std::move(component));
    }
}

void DeviceModelManager21::saveDeviceModel(rapidjson::Document& device_model_doc)
{
    RAPIDJSON_DEFAULT_ALLOCATOR& allocator = device_model_doc.GetAllocator();
    rapidjson::Value             components_json(rapidjson::kArrayType);

    for (const auto& [_, components] : m_device_model.components)
    {
        for (const auto& component : components)
        {
            rapidjson::Value component_json(rapidjson::kObjectType);
            component_json.AddMember(rapidjson::StringRef("name"), rapidjson::Value(component.name.c_str(), allocator).Move(), allocator);
            if (component.evse.isSet())
            {
                component_json.AddMember(rapidjson::StringRef("evse"), rapidjson::Value(component.evse.value()).Move(), allocator);
            }
            if (component.connector.isSet())
            {
                component_json.AddMember(rapidjson::StringRef("connector"), rapidjson::Value(component.connector.value()).Move(), allocator);
            }
            if (component.instance.isSet())
            {
                component_json.AddMember(
                    rapidjson::StringRef("instance"), rapidjson::Value(component.instance.value().c_str(), allocator).Move(), allocator);
            }

            rapidjson::Value variables_json(rapidjson::kArrayType);
            for (const auto& [_, vars] : component.variables)
            {
                for (const auto& [__, var] : vars)
                {
                    rapidjson::Value var_json(rapidjson::kObjectType);
                    var_json.AddMember(rapidjson::StringRef("name"), rapidjson::Value(var.name.c_str(), allocator).Move(), allocator);
                    if (var.instance.isSet())
                    {
                        var_json.AddMember(
                            rapidjson::StringRef("instance"), rapidjson::Value(var.instance.value().c_str(), allocator).Move(), allocator);
                    }

                    rapidjson::Value var_attributes_json(rapidjson::kObjectType);
                    if (var.attributes.type.isSet())
                    {
                        var_attributes_json.AddMember(
                            rapidjson::StringRef("type"),
                            rapidjson::Value(AttributeEnumTypeHelper.toString(var.attributes.type.value()).c_str(), allocator).Move(),
                            allocator);
                    }
                    if (var.attributes.value.isSet())
                    {
                        var_attributes_json.AddMember(rapidjson::StringRef("value"),
                                                      rapidjson::Value(var.attributes.value.value().str().c_str(), allocator).Move(),
                                                      allocator);
                    }
                    if (var.attributes.mutability.isSet())
                    {
                        var_attributes_json.AddMember(
                            rapidjson::StringRef("mutability"),
                            rapidjson::Value(MutabilityEnumTypeHelper.toString(var.attributes.mutability.value()).c_str(), allocator).Move(),
                            allocator);
                    }
                    if (var.attributes.persistent.isSet())
                    {
                        var_attributes_json.AddMember(
                            rapidjson::StringRef("persistent"), rapidjson::Value(var.attributes.persistent.value()).Move(), allocator);
                    }
                    if (var.attributes.constant.isSet())
                    {
                        var_attributes_json.AddMember(
                            rapidjson::StringRef("constant"), rapidjson::Value(var.attributes.constant.value()).Move(), allocator);
                    }
                    var_json.AddMember(rapidjson::StringRef("attributes"), var_attributes_json.Move(), allocator);

                    rapidjson::Value var_chars_json(rapidjson::kObjectType);
                    var_chars_json.AddMember(rapidjson::StringRef("data_type"),
                                             rapidjson::Value(DataEnumTypeHelper.toString(var.characteristics.dataType).c_str(), allocator).Move(),
                                             allocator);
                    if (var.characteristics.unit.isSet())
                    {
                        var_chars_json.AddMember(rapidjson::StringRef("unit"),
                                                 rapidjson::Value(var.characteristics.unit.value().str().c_str(), allocator).Move(),
                                                 allocator);
                    }
                    if (var.characteristics.minLimit.isSet())
                    {
                        var_chars_json.AddMember(
                            rapidjson::StringRef("min_limit"), rapidjson::Value(var.characteristics.minLimit.value()).Move(), allocator);
                    }
                    if (var.characteristics.maxLimit.isSet())
                    {
                        var_chars_json.AddMember(
                            rapidjson::StringRef("max_limit"), rapidjson::Value(var.characteristics.maxLimit.value()).Move(), allocator);
                    }
                    if (var.characteristics.maxElements.isSet())
                    {
                        var_chars_json.AddMember(
                            rapidjson::StringRef("max_elements"), rapidjson::Value(var.characteristics.maxElements.value()).Move(), allocator);
                    }
                    if (var.characteristics.valuesList.isSet())
                    {
                        var_chars_json.AddMember(rapidjson::StringRef("values_list"),
                                                 rapidjson::Value(var.characteristics.valuesList.value().str().c_str(), allocator).Move(),
                                                 allocator);
                    }
                    var_chars_json.AddMember(rapidjson::StringRef("supports_monitoring"),
                                             rapidjson::Value(var.characteristics.supportsMonitoring).Move(),
                                             allocator);
                    var_json.AddMember(rapidjson::StringRef("characteristics"), var_chars_json.Move(), allocator);
                    variables_json.PushBack(var_json.Move(), allocator);
                }
            }
            component_json.AddMember(rapidjson::StringRef("variables"), variables_json.Move(), allocator);
            components_json.PushBack(component_json.Move(), allocator);
        }
    }

    device_model_doc.AddMember(rapidjson::StringRef("components"), components_json.Move(), allocator);
}

DeviceModelComponent21* DeviceModelManager21::getComponent(const ComponentType& requested_component)
{
    return const_cast<DeviceModelComponent21*>(static_cast<const DeviceModelManager21*>(this)->getComponent(requested_component));
}

const DeviceModelComponent21* DeviceModelManager21::getComponent(const ComponentType& requested_component) const
{
    const DeviceModelComponent21* component = nullptr;

    auto iter_comp = m_device_model.components.find(requested_component.name.str());
    if (iter_comp != m_device_model.components.end())
    {
        const std::string requested_instance = optionalString(requested_component.instance);
        for (const auto& comp : iter_comp->second)
        {
            const bool instance_match = !requested_component.instance.isSet() || (requested_instance == optionalString(comp.instance));
            bool evse_match = true;
            if (requested_component.evse.isSet())
            {
                evse_match =
                    comp.evse.isSet() && (requested_component.evse.value().id == static_cast<int>(comp.evse.value())) &&
                    (!requested_component.evse.value().connectorId.isSet() ||
                     (comp.connector.isSet() &&
                      (requested_component.evse.value().connectorId.value() == static_cast<int>(comp.connector.value()))));
            }
            if (instance_match && evse_match)
            {
                component = &comp;
                break;
            }
        }
    }

    return component;
}

DeviceModelVariable21* DeviceModelManager21::getVariable(DeviceModelComponent21&       component,
                                                         const Optional<AttributeEnumType>& attribute,
                                                         const VariableType&                requested_var,
                                                         bool&                              not_supported_attribute_type)
{
    return const_cast<DeviceModelVariable21*>(
        static_cast<const DeviceModelManager21*>(this)->getVariable(component, attribute, requested_var, not_supported_attribute_type));
}

const DeviceModelVariable21* DeviceModelManager21::getVariable(const DeviceModelComponent21&       component,
                                                               const Optional<AttributeEnumType>& attribute,
                                                               const VariableType&                requested_var,
                                                               bool& not_supported_attribute_type) const
{
    const DeviceModelVariable21* var = nullptr;
    not_supported_attribute_type     = false;

    auto iter_var = component.variables.find(requested_var.name.str());
    if (iter_var != component.variables.end())
    {
        auto iter_instance = iter_var->second.find(optionalString(requested_var.instance));
        if (iter_instance != iter_var->second.end())
        {
            AttributeEnumType attribute_type = attribute.isSet() ? attribute.value() : AttributeEnumType::Actual;
            AttributeEnumType variable_type =
                iter_instance->second.attributes.type.isSet() ? iter_instance->second.attributes.type.value() : AttributeEnumType::Actual;
            if (variable_type == attribute_type)
            {
                var = &iter_instance->second;
            }
            else
            {
                not_supported_attribute_type = true;
            }
        }
    }

    return var;
}

bool DeviceModelManager21::isValidValue(const DeviceModelVariable21& var, const std::string& value)
{
    bool ret = true;

    if (var.attributes.mutability.isSet() && (var.attributes.mutability.value() == MutabilityEnumType::ReadOnly))
    {
        m_last_error = "Attempting to modify a Read-Only variable : " + var.name;
        ret          = false;
    }

    if (var.characteristics.minLimit.isSet())
    {
        switch (var.characteristics.dataType)
        {
            case DataEnumType::integer:
            case DataEnumType::decimal:
            {
                float float_value = static_cast<float>(std::atof(value.c_str()));
                if (float_value < var.characteristics.minLimit.value())
                {
                    m_last_error = "Value is below the minimal limit : " + var.name;
                    ret          = false;
                }
            }
            break;
            default:
                break;
        }
    }

    if (var.characteristics.maxLimit.isSet())
    {
        switch (var.characteristics.dataType)
        {
            case DataEnumType::integer:
            case DataEnumType::decimal:
            {
                float float_value = static_cast<float>(std::atof(value.c_str()));
                if (float_value > var.characteristics.maxLimit.value())
                {
                    m_last_error = "Value is above the maximal limit : " + var.name;
                    ret          = false;
                }
            }
            break;
            case DataEnumType::string:
            case DataEnumType::OptionList:
            case DataEnumType::MemberList:
            case DataEnumType::SequenceList:
            {
                size_t max_length = static_cast<size_t>(var.characteristics.maxLimit.value());
                if (value.size() > max_length)
                {
                    m_last_error = "Value length is above the maximal limit : " + var.name;
                    ret          = false;
                }
            }
            break;
            default:
                break;
        }
    }

    if (var.characteristics.valuesList.isSet() && !var.characteristics.valuesList.value().str().empty())
    {
        switch (var.characteristics.dataType)
        {
            case DataEnumType::OptionList:
            case DataEnumType::MemberList:
            case DataEnumType::SequenceList:
            {
                auto allowed_values = ocpp::helpers::split(var.characteristics.valuesList.value().str(), ',');
                bool found = false;
                for (auto& allowed_value : allowed_values)
                {
                    allowed_value = ocpp::helpers::trim(allowed_value);
                    if (value == allowed_value)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    m_last_error = "Value is not in the values list : " + var.name;
                    ret          = false;
                }
            }
            break;
            default:
                break;
        }
    }

    return ret;
}

SetVariableResultType DeviceModelManager21::setVariable(const SetVariableDataType& requested_var, bool check_value)
{
    SetVariableResultType result;
    result.component       = requested_var.component;
    result.variable        = requested_var.variable;
    result.attributeType   = requested_var.attributeType;
    result.attributeStatus = SetVariableStatusEnumType::Rejected;

    DeviceModelComponent21* component = getComponent(requested_var.component);
    if (component)
    {
        bool                   not_supported_attribute_type = false;
        DeviceModelVariable21* var =
            getVariable(*component, requested_var.attributeType, requested_var.variable, not_supported_attribute_type);
        if (var)
        {
            setComponentType(result.component, *component);
            setVariableType(result.variable, *var);
            result.attributeType.value() =
                requested_var.attributeType.isSet() ? requested_var.attributeType.value() : AttributeEnumType::Actual;

            if (!check_value || isValidValue(*var, requested_var.attributeValue.str()))
            {
                if (m_listener)
                {
                    result.attributeStatus = m_listener->setVariable(requested_var);
                }
                else
                {
                    result.attributeStatus = SetVariableStatusEnumType::Accepted;
                }
                if ((result.attributeStatus == SetVariableStatusEnumType::Accepted) ||
                    (result.attributeStatus == SetVariableStatusEnumType::RebootRequired))
                {
                    var->attributes.value.value().assign(requested_var.attributeValue.str());
                }
            }
        }
        else
        {
            result.attributeStatus = not_supported_attribute_type ? SetVariableStatusEnumType::NotSupportedAttributeType
                                                                  : SetVariableStatusEnumType::UnknownVariable;
        }
    }
    else
    {
        result.attributeStatus = SetVariableStatusEnumType::UnknownComponent;
    }

    return result;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
