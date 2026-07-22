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

#ifndef OPENOCPP_OCPP20_ISECURITYMANAGER20_H
#define OPENOCPP_OCPP20_ISECURITYMANAGER20_H

#include "SecurityEventNotification20.h"
#include "SignCertificate20.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 security managers */
class ISecurityManager20
{
  public:
    /** @brief Destructor */
    virtual ~ISecurityManager20() { }

    /** @brief Send a SecurityEventNotification message */
    virtual bool securityEventNotification(const ocpp::messages::ocpp20::SecurityEventNotificationReq& request,
                                           ocpp::messages::ocpp20::SecurityEventNotificationConf&      response,
                                           std::string&                                                error,
                                           std::string&                                                message) = 0;

    /** @brief Send a SignCertificate message */
    virtual bool signCertificate(const ocpp::messages::ocpp20::SignCertificateReq& request,
                                 ocpp::messages::ocpp20::SignCertificateConf&      response,
                                 std::string&                                      error,
                                 std::string&                                      message) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_ISECURITYMANAGER20_H
