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

#ifndef OPENOCPP_OCPP20_IRESERVATIONMANAGER20_H
#define OPENOCPP_OCPP20_IRESERVATIONMANAGER20_H

#include "IdTokenType20.h"

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 reservation managers */
class IReservationManager20
{
  public:
    /** @brief Destructor */
    virtual ~IReservationManager20() { }

    /**
     * @brief Check if a transaction is allowed for an EVSE and token
     * @param evse_id EVSE id
     * @param id_token Id token used to start the transaction
     * @return true if transaction can start, false otherwise
     */
    virtual bool isTransactionAllowed(unsigned int evse_id, const ocpp::types::ocpp20::IdTokenType& id_token) = 0;

    /**
     * @brief Notify that a transaction has started and consume the matching reservation if any
     * @param evse_id EVSE id
     * @param id_token Id token used to start the transaction
     */
    virtual void transactionStarted(unsigned int evse_id, const ocpp::types::ocpp20::IdTokenType& id_token) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_IRESERVATIONMANAGER20_H
