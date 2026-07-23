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

#ifndef OPENOCPP_OCPP21_AUTHENTUTILS21_H
#define OPENOCPP_OCPP21_AUTHENTUTILS21_H

#include "AuthorizationData21.h"
#include "IdTokenInfoType21.h"
#include "IdTokenType21.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

std::string tokenKey(const ocpp::types::ocpp21::IdTokenType& id_token);
std::string tokenType(const ocpp::types::ocpp21::IdTokenType& id_token);

bool serializeIdTokenInfo(const ocpp::types::ocpp21::IdTokenInfoType& token_info, std::string& json);
bool deserializeIdTokenInfo(const std::string& json, ocpp::types::ocpp21::IdTokenInfoType& token_info);

bool serializeAuthorizationData(const ocpp::types::ocpp21::AuthorizationData& authorization_data, std::string& json);
bool deserializeAuthorizationData(const std::string& json, ocpp::types::ocpp21::AuthorizationData& authorization_data);

bool isExpired(const ocpp::types::ocpp21::IdTokenInfoType& token_info);

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_AUTHENTUTILS21_H
