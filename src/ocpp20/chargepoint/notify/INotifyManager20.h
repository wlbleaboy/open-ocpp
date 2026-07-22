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

#ifndef OPENOCPP_OCPP20_INOTIFYMANAGER20_H
#define OPENOCPP_OCPP20_INOTIFYMANAGER20_H

#include "NotifyChargingLimit20.h"
#include "NotifyCustomerInformation20.h"
#include "NotifyDisplayMessages20.h"
#include "NotifyEVChargingNeeds20.h"
#include "NotifyEVChargingSchedule20.h"
#include "NotifyEvent20.h"
#include "NotifyMonitoringReport20.h"
#include "NotifyReport20.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Interface for OCPP 2.0.1 Notify messages managers */
class INotifyManager20
{
  public:
    /** @brief Destructor */
    virtual ~INotifyManager20() { }

    /** @brief Send a NotifyChargingLimit message */
    virtual bool notifyChargingLimit(const ocpp::messages::ocpp20::NotifyChargingLimitReq& request,
                                     ocpp::messages::ocpp20::NotifyChargingLimitConf&      response,
                                     std::string&                                          error,
                                     std::string&                                          message) = 0;

    /** @brief Send a NotifyCustomerInformation message */
    virtual bool notifyCustomerInformation(const ocpp::messages::ocpp20::NotifyCustomerInformationReq& request,
                                           ocpp::messages::ocpp20::NotifyCustomerInformationConf&      response,
                                           std::string&                                                error,
                                           std::string&                                                message) = 0;

    /** @brief Send a NotifyDisplayMessages message */
    virtual bool notifyDisplayMessages(const ocpp::messages::ocpp20::NotifyDisplayMessagesReq& request,
                                       ocpp::messages::ocpp20::NotifyDisplayMessagesConf&      response,
                                       std::string&                                            error,
                                       std::string&                                            message) = 0;

    /** @brief Send a NotifyEVChargingNeeds message */
    virtual bool notifyEVChargingNeeds(const ocpp::messages::ocpp20::NotifyEVChargingNeedsReq& request,
                                       ocpp::messages::ocpp20::NotifyEVChargingNeedsConf&      response,
                                       std::string&                                            error,
                                       std::string&                                            message) = 0;

    /** @brief Send a NotifyEVChargingSchedule message */
    virtual bool notifyEVChargingSchedule(const ocpp::messages::ocpp20::NotifyEVChargingScheduleReq& request,
                                          ocpp::messages::ocpp20::NotifyEVChargingScheduleConf&      response,
                                          std::string&                                               error,
                                          std::string&                                               message) = 0;

    /** @brief Send a NotifyEvent message */
    virtual bool notifyEvent(const ocpp::messages::ocpp20::NotifyEventReq& request,
                             ocpp::messages::ocpp20::NotifyEventConf&      response,
                             std::string&                                  error,
                             std::string&                                  message) = 0;

    /** @brief Send a NotifyMonitoringReport message */
    virtual bool notifyMonitoringReport(const ocpp::messages::ocpp20::NotifyMonitoringReportReq& request,
                                        ocpp::messages::ocpp20::NotifyMonitoringReportConf&      response,
                                        std::string&                                             error,
                                        std::string&                                             message) = 0;

    /** @brief Send a NotifyReport message */
    virtual bool notifyReport(const ocpp::messages::ocpp20::NotifyReportReq& request,
                              ocpp::messages::ocpp20::NotifyReportConf&      response,
                              std::string&                                   error,
                              std::string&                                   message) = 0;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_INOTIFYMANAGER20_H
