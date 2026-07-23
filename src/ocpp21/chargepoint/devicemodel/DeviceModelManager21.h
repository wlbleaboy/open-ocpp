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

#ifndef OPENOCPP_OCPP21_DEVICEMODELMANAGER21_H
#define OPENOCPP_OCPP21_DEVICEMODELMANAGER21_H

#include "IChargePointConfig21.h"
#include "IDeviceModel21.h"
#include "JsonValidator.h"

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Handler for device model operations */
class DeviceModelManager21 : public IDeviceModel21
{
  public:
    /** @brief Constructor */
    DeviceModelManager21(const ocpp::config::IChargePointConfig21& stack_config);

    /** @brief Destructor */
    ~DeviceModelManager21() override;

    /** @brief Initialize the device model loader */
    bool init();

    /** @brief Load the device model from a file */
    bool load(const std::string& device_model_file_path);

    /** @brief Save the device model to a file */
    bool save(const std::string& device_model_file_path);

    /** @brief Load a minimal default model */
    void loadDefaultModel();

    /** @brief Get the last error message */
    const std::string& lastError() const { return m_last_error; }

    void registerListener(IListener& listener) override { m_listener = &listener; }
    const DeviceModel21& getModel() const override { return m_device_model; }
    ocpp::types::ocpp21::GetVariableResultType getVariable(const ocpp::types::ocpp21::GetVariableDataType& requested_var) override;
    ocpp::types::ocpp21::SetVariableResultType setVariable(const ocpp::types::ocpp21::SetVariableDataType& requested_var) override;
    ocpp::types::ocpp21::SetVariableResultType updateVariable(const ocpp::types::ocpp21::SetVariableDataType& requested_var) override;

  private:
    const ocpp::config::IChargePointConfig21& m_stack_config;
    ocpp::json::JsonValidator                 m_validator;
    std::string                               m_last_error;
    DeviceModel21                             m_device_model;
    IListener*                                m_listener;

    void loadDeviceModel(const rapidjson::Document& device_model_doc);
    void saveDeviceModel(rapidjson::Document& device_model_doc);

    DeviceModelComponent21* getComponent(const ocpp::types::ocpp21::ComponentType& requested_component);
    const DeviceModelComponent21* getComponent(const ocpp::types::ocpp21::ComponentType& requested_component) const;
    DeviceModelVariable21* getVariable(DeviceModelComponent21&                                             component,
                                       const ocpp::types::Optional<ocpp::types::ocpp21::AttributeEnumType>& attribute,
                                       const ocpp::types::ocpp21::VariableType&                             requested_var,
                                       bool&                                                                not_supported_attribute_type);
    const DeviceModelVariable21* getVariable(const DeviceModelComponent21&                                       component,
                                             const ocpp::types::Optional<ocpp::types::ocpp21::AttributeEnumType>& attribute,
                                             const ocpp::types::ocpp21::VariableType&                             requested_var,
                                             bool& not_supported_attribute_type) const;
    bool isValidValue(const DeviceModelVariable21& var, const std::string& value);
    ocpp::types::ocpp21::SetVariableResultType setVariable(const ocpp::types::ocpp21::SetVariableDataType& requested_var,
                                                           bool                                           check_value);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_DEVICEMODELMANAGER21_H
