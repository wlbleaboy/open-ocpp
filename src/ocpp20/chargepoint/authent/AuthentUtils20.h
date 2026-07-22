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

#ifndef OPENOCPP_OCPP20_AUTHENTUTILS20_H
#define OPENOCPP_OCPP20_AUTHENTUTILS20_H

#include "AuthorizationData20.h"
#include "IdTokenEnumType20.h"
#include "IdTokenInfoType20.h"
#include "IdTokenType20.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

std::string tokenKey(const ocpp::types::ocpp20::IdTokenType& id_token);
std::string tokenType(const ocpp::types::ocpp20::IdTokenType& id_token);

bool serializeIdTokenInfo(const ocpp::types::ocpp20::IdTokenInfoType& token_info, std::string& json);
bool deserializeIdTokenInfo(const std::string& json, ocpp::types::ocpp20::IdTokenInfoType& token_info);

bool serializeAuthorizationData(const ocpp::types::ocpp20::AuthorizationData& authorization_data, std::string& json);
bool deserializeAuthorizationData(const std::string& json, ocpp::types::ocpp20::AuthorizationData& authorization_data);

bool isExpired(const ocpp::types::ocpp20::IdTokenInfoType& token_info);

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_AUTHENTUTILS20_H
