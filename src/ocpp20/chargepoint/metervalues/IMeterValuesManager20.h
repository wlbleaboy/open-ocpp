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

#ifndef OPENOCPP_OCPP20_IMETERVALUESMANAGER20_H
#define OPENOCPP_OCPP20_IMETERVALUESMANAGER20_H

#include "MeterValueType20.h"
#include "ReadingContextEnumType20.h"

#include <chrono>
#include <string>
#include <vector>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 charge point meter values managers */
class IMeterValuesManager20
{
  public:
    /** @brief Destructor */
    virtual ~IMeterValuesManager20() { }

    /** @brief Send meter values for an EVSE */
    virtual bool sendMeterValues(unsigned int evse_id, const std::vector<ocpp::types::ocpp20::MeterValueType>& values) = 0;

    /** @brief Start periodic MeterValues sampling for an EVSE */
    virtual void startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval) = 0;

    /** @brief Stop periodic MeterValues sampling for an EVSE */
    virtual void stopPeriodicMeterValues(unsigned int evse_id) = 0;

    /** @brief Start transaction sampled meter values */
    virtual void startTransactionSampledMeterValues(const std::string& transaction_id,
                                                    unsigned int       evse_id,
                                                    std::chrono::seconds interval) = 0;

    /** @brief Stop transaction sampled meter values */
    virtual void stopTransactionSampledMeterValues(const std::string& transaction_id) = 0;

    /** @brief Get stored stop transaction sampled values */
    virtual void getTxStopMeterValues(const std::string& transaction_id,
                                      std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_IMETERVALUESMANAGER20_H
