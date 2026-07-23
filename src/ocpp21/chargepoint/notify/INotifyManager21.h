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

#ifndef OPENOCPP_OCPP21_INOTIFYMANAGER21_H
#define OPENOCPP_OCPP21_INOTIFYMANAGER21_H

#include "NotifyAllowedEnergyTransfer21.h"
#include "NotifyChargingLimit21.h"
#include "NotifyCustomerInformation21.h"
#include "NotifyDERAlarm21.h"
#include "NotifyDERStartStop21.h"
#include "NotifyDisplayMessages21.h"
#include "NotifyEVChargingNeeds21.h"
#include "NotifyEVChargingSchedule21.h"
#include "NotifyEvent21.h"
#include "NotifyMonitoringReport21.h"
#include "NotifyPeriodicEventStream21.h"
#include "NotifyPriorityCharging21.h"
#include "NotifyReport21.h"
#include "NotifySettlement21.h"
#include "NotifyWebPaymentStarted21.h"

#include <string>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Interface for OCPP 2.1 Notify messages managers */
class INotifyManager21
{
  public:
    virtual ~INotifyManager21() { }

    virtual bool notifyAllowedEnergyTransfer(const ocpp::messages::ocpp21::NotifyAllowedEnergyTransferReq& request,
                                             ocpp::messages::ocpp21::NotifyAllowedEnergyTransferConf&      response,
                                             std::string&                                                  error,
                                             std::string&                                                  message) = 0;
    virtual bool notifyChargingLimit(const ocpp::messages::ocpp21::NotifyChargingLimitReq& request,
                                     ocpp::messages::ocpp21::NotifyChargingLimitConf&      response,
                                     std::string&                                          error,
                                     std::string&                                          message) = 0;
    virtual bool notifyCustomerInformation(const ocpp::messages::ocpp21::NotifyCustomerInformationReq& request,
                                           ocpp::messages::ocpp21::NotifyCustomerInformationConf&      response,
                                           std::string&                                                error,
                                           std::string&                                                message) = 0;
    virtual bool notifyDERAlarm(const ocpp::messages::ocpp21::NotifyDERAlarmReq& request,
                                ocpp::messages::ocpp21::NotifyDERAlarmConf&      response,
                                std::string&                                     error,
                                std::string&                                     message) = 0;
    virtual bool notifyDERStartStop(const ocpp::messages::ocpp21::NotifyDERStartStopReq& request,
                                    ocpp::messages::ocpp21::NotifyDERStartStopConf&      response,
                                    std::string&                                         error,
                                    std::string&                                         message) = 0;
    virtual bool notifyDisplayMessages(const ocpp::messages::ocpp21::NotifyDisplayMessagesReq& request,
                                       ocpp::messages::ocpp21::NotifyDisplayMessagesConf&      response,
                                       std::string&                                            error,
                                       std::string&                                            message) = 0;
    virtual bool notifyEVChargingNeeds(const ocpp::messages::ocpp21::NotifyEVChargingNeedsReq& request,
                                       ocpp::messages::ocpp21::NotifyEVChargingNeedsConf&      response,
                                       std::string&                                            error,
                                       std::string&                                            message) = 0;
    virtual bool notifyEVChargingSchedule(const ocpp::messages::ocpp21::NotifyEVChargingScheduleReq& request,
                                          ocpp::messages::ocpp21::NotifyEVChargingScheduleConf&      response,
                                          std::string&                                               error,
                                          std::string&                                               message) = 0;
    virtual bool notifyEvent(const ocpp::messages::ocpp21::NotifyEventReq& request,
                             ocpp::messages::ocpp21::NotifyEventConf&      response,
                             std::string&                                  error,
                             std::string&                                  message) = 0;
    virtual bool notifyMonitoringReport(const ocpp::messages::ocpp21::NotifyMonitoringReportReq& request,
                                        ocpp::messages::ocpp21::NotifyMonitoringReportConf&      response,
                                        std::string&                                             error,
                                        std::string&                                             message) = 0;
    virtual bool notifyPeriodicEventStream(const ocpp::messages::ocpp21::NotifyPeriodicEventStreamReq& request,
                                           ocpp::messages::ocpp21::NotifyPeriodicEventStreamConf&      response,
                                           std::string&                                                error,
                                           std::string&                                                message) = 0;
    virtual bool notifyPriorityCharging(const ocpp::messages::ocpp21::NotifyPriorityChargingReq& request,
                                        ocpp::messages::ocpp21::NotifyPriorityChargingConf&      response,
                                        std::string&                                             error,
                                        std::string&                                             message) = 0;
    virtual bool notifyReport(const ocpp::messages::ocpp21::NotifyReportReq& request,
                              ocpp::messages::ocpp21::NotifyReportConf&      response,
                              std::string&                                   error,
                              std::string&                                   message) = 0;
    virtual bool notifySettlement(const ocpp::messages::ocpp21::NotifySettlementReq& request,
                                  ocpp::messages::ocpp21::NotifySettlementConf&      response,
                                  std::string&                                       error,
                                  std::string&                                       message) = 0;
    virtual bool notifyWebPaymentStarted(const ocpp::messages::ocpp21::NotifyWebPaymentStartedReq& request,
                                         ocpp::messages::ocpp21::NotifyWebPaymentStartedConf&      response,
                                         std::string&                                              error,
                                         std::string&                                              message) = 0;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_INOTIFYMANAGER21_H
