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

#ifndef OPENOCPP_OCPP20_ISMARTCHARGINGMANAGER20_H
#define OPENOCPP_OCPP20_ISMARTCHARGINGMANAGER20_H

#include "ClearedChargingLimit20.h"
#include "ChargingProfileType20.h"
#include "ChargingRateUnitEnumType20.h"
#include "ChargingSchedulePeriodType20.h"
#include "Optional.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for the OCPP 2.0.1 smart charging manager of the charge point */
class ISmartChargingManager20
{
  public:
    /** @brief Smart charging setpoint extracted from the active schedule */
    struct SmartChargingSetpoint
    {
        /** @brief Rate limit value */
        float limit;
        /** @brief Rate limit unit */
        ocpp::types::ocpp20::ChargingRateUnitEnumType unit;
        /** @brief Number of phases */
        ocpp::types::Optional<int> numberPhases;
        /** @brief Phase to use */
        ocpp::types::Optional<int> phaseToUse;
    };

    /** @brief Destructor */
    virtual ~ISmartChargingManager20() { }

    /**
     * @brief Send a ClearedChargingLimit message
     * @param request Request to send
     * @param response Received response
     * @param error Error (Empty if not a CallError)
     * @param message Error message (Empty if not a CallError)
     * @return true if the request has been sent and a response has been received, false otherwise
     */
    virtual bool clearedChargingLimit(const ocpp::messages::ocpp20::ClearedChargingLimitReq& request,
                                      ocpp::messages::ocpp20::ClearedChargingLimitConf&      response,
                                      std::string&                                           error,
                                      std::string&                                           message) = 0;

    /**
     * @brief Get the active smart charging setpoints for an EVSE and for the whole charging station
     * @param evse_id Id of the EVSE
     * @param charging_station_setpoint Charging station setpoint, if any
     * @param evse_setpoint EVSE setpoint, if any
     * @param unit Expected setpoint unit
     * @return true if at least one setpoint has been computed, false otherwise
     */
    virtual bool getSetpoint(unsigned int                                         evse_id,
                             ocpp::types::Optional<SmartChargingSetpoint>&        charging_station_setpoint,
                             ocpp::types::Optional<SmartChargingSetpoint>&        evse_setpoint,
                             ocpp::types::ocpp20::ChargingRateUnitEnumType unit = ocpp::types::ocpp20::ChargingRateUnitEnumType::A) = 0;

    /**
     * @brief Install a TxProfile charging profile on an EVSE
     * @param evse_id Id of the EVSE targeted by the charging profile
     * @param profile Charging profile to install
     * @return true if the charging profile has been installed, false otherwise
     */
    virtual bool installTxProfile(unsigned int evse_id, const ocpp::types::ocpp20::ChargingProfileType& profile) = 0;

    /**
     * @brief Assign the pending TxProfile of an EVSE to a transaction
     * @param evse_id Id of the EVSE targeted by the charging profile
     * @param transaction_id Transaction to associate with the profile
     */
    virtual void assignPendingTxProfiles(unsigned int evse_id, const std::string& transaction_id) = 0;

    /**
     * @brief Clear all the TxProfile charging profiles on an EVSE
     * @param evse_id Id of the EVSE
     */
    virtual void clearTxProfiles(unsigned int evse_id) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_ISMARTCHARGINGMANAGER20_H
