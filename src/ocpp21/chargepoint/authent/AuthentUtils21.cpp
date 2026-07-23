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

#include "AuthentUtils21.h"

#include "Logger.h"

#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

std::string tokenKey(const IdTokenType& id_token)
{
    return id_token.idToken.str();
}

std::string tokenType(const IdTokenType& id_token)
{
    return id_token.type.str();
}

bool serializeIdTokenInfo(const IdTokenInfoType& token_info, std::string& json)
{
    bool                     ret = false;
    IdTokenInfoTypeConverter converter;
    rapidjson::Document      doc;
    doc.SetObject();
    converter.setAllocator(&doc.GetAllocator());
    if (converter.toJson(token_info, doc))
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        json = buffer.GetString();
        ret  = true;
    }
    return ret;
}

bool deserializeIdTokenInfo(const std::string& json, IdTokenInfoType& token_info)
{
    bool                ret = false;
    rapidjson::Document doc;
    if (!doc.Parse(json.c_str()).HasParseError() && doc.IsObject())
    {
        std::string              error_code;
        std::string              error_message;
        IdTokenInfoTypeConverter converter;
        ret = converter.fromJson(doc, token_info, error_code, error_message);
        if (!ret)
        {
            LOG_ERROR << "Unable to parse cached idTokenInfo : " << error_code << " - " << error_message;
        }
    }
    return ret;
}

bool serializeAuthorizationData(const AuthorizationData& authorization_data, std::string& json)
{
    bool                       ret = false;
    AuthorizationDataConverter converter;
    rapidjson::Document        doc;
    doc.SetObject();
    converter.setAllocator(&doc.GetAllocator());
    if (converter.toJson(authorization_data, doc))
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        json = buffer.GetString();
        ret  = true;
    }
    return ret;
}

bool deserializeAuthorizationData(const std::string& json, AuthorizationData& authorization_data)
{
    bool                ret = false;
    rapidjson::Document doc;
    if (!doc.Parse(json.c_str()).HasParseError() && doc.IsObject())
    {
        std::string                error_code;
        std::string                error_message;
        AuthorizationDataConverter converter;
        ret = converter.fromJson(doc, authorization_data, error_code, error_message);
        if (!ret)
        {
            LOG_ERROR << "Unable to parse local authorization data : " << error_code << " - " << error_message;
        }
    }
    return ret;
}

bool isExpired(const IdTokenInfoType& token_info)
{
    bool ret = false;
    if (token_info.cacheExpiryDateTime.isSet())
    {
        const std::time_t expiry = token_info.cacheExpiryDateTime.value().timestamp();
        const std::time_t now    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ret                      = (expiry < now);
    }
    return ret;
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
