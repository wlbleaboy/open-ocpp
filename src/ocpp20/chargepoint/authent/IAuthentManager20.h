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

#ifndef OPENOCPP_OCPP20_IAUTHENTMANAGER20_H
#define OPENOCPP_OCPP20_IAUTHENTMANAGER20_H

#include "IdTokenInfoType20.h"
#include "IdTokenType20.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 charge point authentication managers */
class IAuthentManager20
{
  public:
    /** @brief Destructor */
    virtual ~IAuthentManager20() { }

    /**
     * @brief Authorize an id token
     * @param id_token Id token to authorize
     * @param token_info Authorization information
     * @param error Error (Empty if not a CallError)
     * @param message Error message (Empty if not a CallError)
     * @return true if an authorization status has been found, false otherwise
     */
    virtual bool authorize(const ocpp::types::ocpp20::IdTokenType& id_token,
                           ocpp::types::ocpp20::IdTokenInfoType&  token_info,
                           std::string&                           error,
                           std::string&                           message) = 0;

    /**
     * @brief Update the local authorization cache
     * @param id_token Id token to update
     * @param token_info Authorization information
     */
    virtual void update(const ocpp::types::ocpp20::IdTokenType& id_token, const ocpp::types::ocpp20::IdTokenInfoType& token_info) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_IAUTHENTMANAGER20_H
