/*
MIT License

Copyright (c) 2020 Cedric Jimenez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CHARGEPOINTDEMOCONFIG_H
#define CHARGEPOINTDEMOCONFIG_H

#include "ChargePointConfig.h"
#include "IDeviceModel21.h"
#include "IniFile.h"

/** @brief Configuration of the Charge Point demo */
class ChargePointDemoConfig
{
  public:
    ChargePointDemoConfig(const std::string& config_file) : m_config(config_file), m_stack_config(m_config) { }

    ocpp::config::IChargePointConfig21& stackConfig() { return m_stack_config; }

    void setStackConfigValue(const std::string& key, const std::string& value) { m_stack_config.setConfigValue(key, value); }

    ocpp::helpers::IniFile::Value getConfig(const std::string& section, const std::string& key)
    {
        return m_config.get(section, key, "");
    }

    bool getDeviceModelValue(const ocpp::types::ocpp21::ComponentType& component,
                             const ocpp::types::ocpp21::VariableType&  variable,
                             std::string&                              value)
    {
        const std::string variable_name = buildVariableName(variable);
        const std::string component_name = buildComponentName(component);
        value = m_config.get(component_name, variable_name).toString();
        if (value.empty())
        {
            const std::string legacy_component_name = buildLegacyComponentName(component);
            if (legacy_component_name != component_name)
            {
                value = m_config.get(legacy_component_name, variable_name).toString();
            }
        }
        return true;
    }

    bool setDeviceModelValue(const ocpp::types::ocpp21::ComponentType& component,
                             const ocpp::types::ocpp21::VariableType&  variable,
                             const std::string&                        value)
    {
        m_config.set(buildComponentName(component), buildVariableName(variable), value);
        m_config.store();
        return true;
    }

    bool save() { return m_config.store(); }

  private:
    ocpp::helpers::IniFile m_config;
    ChargePointConfig      m_stack_config;

    std::string buildComponentName(const ocpp::types::ocpp21::ComponentType& component)
    {
        std::string name = component.name.str();
        if (component.instance.isSet())
        {
            name += "." + component.instance.value().str();
        }
        if (component.evse.isSet())
        {
            name += "." + std::to_string(component.evse.value().id);
            if (component.evse.value().connectorId.isSet())
            {
                name += "." + std::to_string(component.evse.value().connectorId.value());
            }
        }
        return name;
    }

    std::string buildLegacyComponentName(const ocpp::types::ocpp21::ComponentType& component)
    {
        std::string name = component.name.str();
        if (component.instance.isSet())
        {
            name += "." + legacyInstanceName(component.instance.value().str());
        }
        if (component.evse.isSet())
        {
            name += "." + std::to_string(component.evse.value().id);
            if (component.evse.value().connectorId.isSet())
            {
                name += "." + std::to_string(component.evse.value().connectorId.value());
            }
        }
        return name;
    }

    std::string legacyInstanceName(const std::string& instance)
    {
        size_t digits_pos = instance.size();
        while ((digits_pos > 0u) && (instance[digits_pos - 1u] >= '0') && (instance[digits_pos - 1u] <= '9'))
        {
            --digits_pos;
        }
        if ((digits_pos > 0u) && (digits_pos < instance.size()) && (instance[digits_pos - 1u] != ' '))
        {
            return instance.substr(0u, digits_pos) + " " + instance.substr(digits_pos);
        }
        return instance;
    }

    std::string buildVariableName(const ocpp::types::ocpp21::VariableType& variable)
    {
        std::string name = variable.name.str();
        if (variable.instance.isSet())
        {
            name += "." + variable.instance.value().str();
        }
        return name;
    }
};

#endif // CHARGEPOINTDEMOCONFIG_H
