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

#ifndef OPENOCPP_OCPP20_ITRANSACTIONMANAGER20_H
#define OPENOCPP_OCPP20_ITRANSACTIONMANAGER20_H

#include "CostUpdated20.h"
#include "IdTokenType20.h"
#include "MeterValueType20.h"
#include "ReasonEnumType20.h"
#include "TriggerReasonEnumType20.h"

#include <string>
#include <vector>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 charge point transaction managers */
class ITransactionManager20
{
  public:
    /** @brief Destructor */
    virtual ~ITransactionManager20() { }

    /**
     * @brief Send a CostUpdated message
     * @param request Request to send
     * @param response Received response
     * @param error Error (Empty if not a CallError)
     * @param message Error message (Empty if not a CallError)
     * @return true if the request has been sent and a response has been received, false otherwise
     */
    virtual bool costUpdated(const ocpp::messages::ocpp20::CostUpdatedReq& request,
                             ocpp::messages::ocpp20::CostUpdatedConf&      response,
                             std::string&                                  error,
                             std::string&                                  message) = 0;

    /** @brief Start a transaction */
    virtual bool startTransaction(unsigned int                            evse_id,
                                  unsigned int                            connector_id,
                                  const ocpp::types::ocpp20::IdTokenType& id_token,
                                  ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                                  std::string&                            transaction_id) = 0;

    /** @brief Start a transaction with an optional remote start id */
    virtual bool startTransaction(unsigned int                                evse_id,
                                  unsigned int                                connector_id,
                                  const ocpp::types::ocpp20::IdTokenType&     id_token,
                                  ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                                  int                                         remote_start_id,
                                  std::string&                                transaction_id) = 0;

    /** @brief Send a transaction update */
    virtual bool updateTransaction(const std::string&                                         transaction_id,
                                   ocpp::types::ocpp20::TriggerReasonEnumType                 trigger_reason,
                                   const std::vector<ocpp::types::ocpp20::MeterValueType>&    meter_values) = 0;

    /** @brief Check if a transaction is active */
    virtual bool hasActiveTransaction(const std::string& transaction_id) = 0;

    /** @brief Check if a transaction is active on an EVSE connector */
    virtual bool hasActiveTransaction(unsigned int evse_id, unsigned int connector_id) = 0;

    /** @brief Stop a transaction */
    virtual bool stopTransaction(const std::string&                         transaction_id,
                                 ocpp::types::ocpp20::ReasonEnumType       reason,
                                 ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                                 const ocpp::types::ocpp20::IdTokenType*   id_token,
                                 const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_ITRANSACTIONMANAGER20_H
