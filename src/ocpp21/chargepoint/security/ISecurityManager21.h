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

#ifndef OPENOCPP_OCPP21_ISECURITYMANAGER21_H
#define OPENOCPP_OCPP21_ISECURITYMANAGER21_H

#include "SecurityEventNotification21.h"
#include "SignCertificate21.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Interface for OCPP 2.1 security managers */
class ISecurityManager21
{
  public:
    virtual ~ISecurityManager21() { }

    virtual bool securityEventNotification(const ocpp::messages::ocpp21::SecurityEventNotificationReq& request,
                                           ocpp::messages::ocpp21::SecurityEventNotificationConf&      response,
                                           std::string&                                                error,
                                           std::string&                                                message) = 0;

    virtual bool signCertificate(const ocpp::messages::ocpp21::SignCertificateReq& request,
                                 ocpp::messages::ocpp21::SignCertificateConf&      response,
                                 std::string&                                      error,
                                 std::string&                                      message) = 0;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_ISECURITYMANAGER21_H
