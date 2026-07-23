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

#ifndef OPENOCPP_OCPP21_ITRIGGERMESSAGEMANAGER21_H
#define OPENOCPP_OCPP21_ITRIGGERMESSAGEMANAGER21_H

#include "EVSEType21.h"
#include "MessageTriggerEnumType21.h"
#include "Optional.h"

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Interface for OCPP 2.1 TriggerMessage managers */
class ITriggerMessageManager21
{
  public:
    class ITriggerMessageHandler;

    virtual ~ITriggerMessageManager21() { }

    virtual void registerHandler(ocpp::types::ocpp21::MessageTriggerEnumType message, ITriggerMessageHandler& handler) = 0;

    /** @brief Interface for trigger message handlers implementations */
    class ITriggerMessageHandler
    {
      public:
        virtual ~ITriggerMessageHandler() { }

        virtual bool onTriggerMessage(ocpp::types::ocpp21::MessageTriggerEnumType                 message,
                                      const ocpp::types::Optional<ocpp::types::ocpp21::EVSEType>& evse) = 0;
    };
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_ITRIGGERMESSAGEMANAGER21_H
