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

#include "NotifyManager20.h"

#include "GenericMessageSender.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
NotifyManager20::NotifyManager20(ocpp::messages::GenericMessageSender& msg_sender) : m_msg_sender(msg_sender) { }

/** @brief Destructor */
NotifyManager20::~NotifyManager20() { }

/** @brief Send a NotifyChargingLimit message */
bool NotifyManager20::notifyChargingLimit(const NotifyChargingLimitReq& request,
                                          NotifyChargingLimitConf&      response,
                                          std::string&                  error,
                                          std::string&                  message)
{
    return call(NOTIFYCHARGINGLIMIT_ACTION, request, response, error, message);
}

/** @brief Send a NotifyCustomerInformation message */
bool NotifyManager20::notifyCustomerInformation(const NotifyCustomerInformationReq& request,
                                                NotifyCustomerInformationConf&      response,
                                                std::string&                        error,
                                                std::string&                        message)
{
    return call(NOTIFYCUSTOMERINFORMATION_ACTION, request, response, error, message);
}

/** @brief Send a NotifyDisplayMessages message */
bool NotifyManager20::notifyDisplayMessages(const NotifyDisplayMessagesReq& request,
                                            NotifyDisplayMessagesConf&      response,
                                            std::string&                    error,
                                            std::string&                    message)
{
    return call(NOTIFYDISPLAYMESSAGES_ACTION, request, response, error, message);
}

/** @brief Send a NotifyEVChargingNeeds message */
bool NotifyManager20::notifyEVChargingNeeds(const NotifyEVChargingNeedsReq& request,
                                            NotifyEVChargingNeedsConf&      response,
                                            std::string&                    error,
                                            std::string&                    message)
{
    return call(NOTIFYEVCHARGINGNEEDS_ACTION, request, response, error, message);
}

/** @brief Send a NotifyEVChargingSchedule message */
bool NotifyManager20::notifyEVChargingSchedule(const NotifyEVChargingScheduleReq& request,
                                               NotifyEVChargingScheduleConf&      response,
                                               std::string&                       error,
                                               std::string&                       message)
{
    return call(NOTIFYEVCHARGINGSCHEDULE_ACTION, request, response, error, message);
}

/** @brief Send a NotifyEvent message */
bool NotifyManager20::notifyEvent(const NotifyEventReq& request,
                                  NotifyEventConf&      response,
                                  std::string&          error,
                                  std::string&          message)
{
    return call(NOTIFYEVENT_ACTION, request, response, error, message);
}

/** @brief Send a NotifyMonitoringReport message */
bool NotifyManager20::notifyMonitoringReport(const NotifyMonitoringReportReq& request,
                                             NotifyMonitoringReportConf&      response,
                                             std::string&                     error,
                                             std::string&                     message)
{
    return call(NOTIFYMONITORINGREPORT_ACTION, request, response, error, message);
}

/** @brief Send a NotifyReport message */
bool NotifyManager20::notifyReport(const NotifyReportReq& request,
                                   NotifyReportConf&      response,
                                   std::string&           error,
                                   std::string&           message)
{
    return call(NOTIFYREPORT_ACTION, request, response, error, message);
}

/** @brief Execute a notify call */
template <typename RequestType, typename ResponseType>
bool NotifyManager20::call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message)
{
    bool ret = false;

    CallResult result = m_msg_sender.call(action, request, response, error, message);
    if (result == CallResult::Ok)
    {
        ret = true;
    }
    else
    {
        LOG_ERROR << action << " => " << (result == CallResult::Failed ? "Timeout" : "Error");
    }

    return ret;
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
