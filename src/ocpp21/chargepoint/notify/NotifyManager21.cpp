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

#include "NotifyManager21.h"

#include "GenericMessageSender.h"
#include "Logger.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

NotifyManager21::NotifyManager21(ocpp::messages::GenericMessageSender& msg_sender) : m_msg_sender(msg_sender) { }

NotifyManager21::~NotifyManager21() { }

bool NotifyManager21::notifyAllowedEnergyTransfer(const NotifyAllowedEnergyTransferReq& request,
                                                  NotifyAllowedEnergyTransferConf&      response,
                                                  std::string&                          error,
                                                  std::string&                          message)
{
    return call(NOTIFYALLOWEDENERGYTRANSFER_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyChargingLimit(const NotifyChargingLimitReq& request,
                                          NotifyChargingLimitConf&      response,
                                          std::string&                  error,
                                          std::string&                  message)
{
    return call(NOTIFYCHARGINGLIMIT_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyCustomerInformation(const NotifyCustomerInformationReq& request,
                                                NotifyCustomerInformationConf&      response,
                                                std::string&                        error,
                                                std::string&                        message)
{
    return call(NOTIFYCUSTOMERINFORMATION_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyDERAlarm(const NotifyDERAlarmReq& request,
                                     NotifyDERAlarmConf&      response,
                                     std::string&             error,
                                     std::string&             message)
{
    return call(NOTIFYDERALARM_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyDERStartStop(const NotifyDERStartStopReq& request,
                                         NotifyDERStartStopConf&      response,
                                         std::string&                 error,
                                         std::string&                 message)
{
    return call(NOTIFYDERSTARTSTOP_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyDisplayMessages(const NotifyDisplayMessagesReq& request,
                                            NotifyDisplayMessagesConf&      response,
                                            std::string&                    error,
                                            std::string&                    message)
{
    return call(NOTIFYDISPLAYMESSAGES_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyEVChargingNeeds(const NotifyEVChargingNeedsReq& request,
                                            NotifyEVChargingNeedsConf&      response,
                                            std::string&                    error,
                                            std::string&                    message)
{
    return call(NOTIFYEVCHARGINGNEEDS_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyEVChargingSchedule(const NotifyEVChargingScheduleReq& request,
                                               NotifyEVChargingScheduleConf&      response,
                                               std::string&                       error,
                                               std::string&                       message)
{
    return call(NOTIFYEVCHARGINGSCHEDULE_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyEvent(const NotifyEventReq& request,
                                  NotifyEventConf&      response,
                                  std::string&          error,
                                  std::string&          message)
{
    return call(NOTIFYEVENT_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyMonitoringReport(const NotifyMonitoringReportReq& request,
                                             NotifyMonitoringReportConf&      response,
                                             std::string&                     error,
                                             std::string&                     message)
{
    return call(NOTIFYMONITORINGREPORT_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyPeriodicEventStream(const NotifyPeriodicEventStreamReq& request,
                                                NotifyPeriodicEventStreamConf&      response,
                                                std::string&                        error,
                                                std::string&                        message)
{
    return call(NOTIFYPERIODICEVENTSTREAM_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyPriorityCharging(const NotifyPriorityChargingReq& request,
                                             NotifyPriorityChargingConf&      response,
                                             std::string&                     error,
                                             std::string&                     message)
{
    return call(NOTIFYPRIORITYCHARGING_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyReport(const NotifyReportReq& request,
                                   NotifyReportConf&      response,
                                   std::string&           error,
                                   std::string&           message)
{
    return call(NOTIFYREPORT_ACTION, request, response, error, message);
}

bool NotifyManager21::notifySettlement(const NotifySettlementReq& request,
                                       NotifySettlementConf&      response,
                                       std::string&               error,
                                       std::string&               message)
{
    return call(NOTIFYSETTLEMENT_ACTION, request, response, error, message);
}

bool NotifyManager21::notifyWebPaymentStarted(const NotifyWebPaymentStartedReq& request,
                                              NotifyWebPaymentStartedConf&      response,
                                              std::string&                      error,
                                              std::string&                      message)
{
    return call(NOTIFYWEBPAYMENTSTARTED_ACTION, request, response, error, message);
}

template <typename RequestType, typename ResponseType>
bool NotifyManager21::call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message)
{
    const CallResult result = m_msg_sender.call(action, request, response, error, message);
    if (result != CallResult::Ok)
    {
        LOG_ERROR << action << " => " << (result == CallResult::Failed ? "Timeout" : "Error");
    }
    return (result == CallResult::Ok);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
