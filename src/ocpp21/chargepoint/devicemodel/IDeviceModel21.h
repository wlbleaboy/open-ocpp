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

#ifndef OPENOCPP_OCPP21_IDEVICEMODEL21_H
#define OPENOCPP_OCPP21_IDEVICEMODEL21_H

#include "GetVariableDataType21.h"
#include "GetVariableResultType21.h"
#include "SetVariableDataType21.h"
#include "SetVariableResultType21.h"
#include "VariableAttributeType21.h"
#include "VariableCharacteristicsType21.h"

#include <map>
#include <string>
#include <vector>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Variable */
struct DeviceModelVariable21
{
    /** @brief Name */
    std::string name;
    /** @brief Instance */
    ocpp::types::Optional<std::string> instance;
    /** @brief Attributes */
    ocpp::types::ocpp21::VariableAttributeType attributes;
    /** @brief Characteristics */
    ocpp::types::ocpp21::VariableCharacteristicsType characteristics;
};

/** @brief Component */
struct DeviceModelComponent21
{
    /** @brief Name */
    std::string name;
    /** @brief Instance */
    ocpp::types::Optional<std::string> instance;
    /** @brief EVSE id */
    ocpp::types::Optional<unsigned int> evse;
    /** @brief Connector id */
    ocpp::types::Optional<unsigned int> connector;
    /** @brief Variables */
    std::map<std::string, std::map<std::string, DeviceModelVariable21>> variables;
};

/** @brief Device model */
struct DeviceModel21
{
    /** @brief Components in the device model */
    std::map<std::string, std::vector<DeviceModelComponent21>> components;
};

/** @brief Interface to interact with the device model */
class IDeviceModel21
{
  public:
    class IListener;

    /** @brief Destructor */
    virtual ~IDeviceModel21() { }

    /** @brief Register a listener to device model events */
    virtual void registerListener(IListener& listener) = 0;

    /** @brief Get the full device model */
    virtual const DeviceModel21& getModel() const = 0;

    /** @brief Get a variable value in the device model */
    virtual ocpp::types::ocpp21::GetVariableResultType getVariable(const ocpp::types::ocpp21::GetVariableDataType& requested_var) = 0;

    /** @brief Set a variable value in the device model */
    virtual ocpp::types::ocpp21::SetVariableResultType setVariable(const ocpp::types::ocpp21::SetVariableDataType& requested_var) = 0;

    /** @brief Update a variable value in the device model without value or mutability check */
    virtual ocpp::types::ocpp21::SetVariableResultType updateVariable(const ocpp::types::ocpp21::SetVariableDataType& requested_var) = 0;

    /** @brief Interface to the listeners of the device model events */
    class IListener
    {
      public:
        /** @brief Destructor */
        virtual ~IListener() { }

        /** @brief Called to retrieve the value of a variable */
        virtual void getVariable(ocpp::types::ocpp21::GetVariableResultType& var) = 0;
        /** @brief Called to set the value of a variable */
        virtual ocpp::types::ocpp21::SetVariableStatusEnumType setVariable(
            const ocpp::types::ocpp21::SetVariableDataType& var) = 0;
    };
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_IDEVICEMODEL21_H
