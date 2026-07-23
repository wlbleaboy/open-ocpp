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

#ifndef OPENOCPP_OCPP21_IRESERVATIONMANAGER21_H
#define OPENOCPP_OCPP21_IRESERVATIONMANAGER21_H

#include "IdTokenType21.h"

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Interface for OCPP 2.1 reservation managers */
class IReservationManager21
{
  public:
    virtual ~IReservationManager21() { }

    virtual bool isTransactionAllowed(unsigned int evse_id, const ocpp::types::ocpp21::IdTokenType& id_token) = 0;
    virtual void transactionStarted(unsigned int evse_id, const ocpp::types::ocpp21::IdTokenType& id_token) = 0;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_IRESERVATIONMANAGER21_H
