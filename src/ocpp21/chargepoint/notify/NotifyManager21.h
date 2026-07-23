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

#ifndef OPENOCPP_OCPP21_NOTIFYMANAGER21_H
#define OPENOCPP_OCPP21_NOTIFYMANAGER21_H

#include "INotifyManager21.h"

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
} // namespace messages

namespace chargepoint
{
namespace ocpp21
{

/** @brief Handle OCPP 2.1 Notify* outbound messages */
class NotifyManager21 : public INotifyManager21
{
  public:
    NotifyManager21(ocpp::messages::GenericMessageSender& msg_sender);
    virtual ~NotifyManager21();

    bool notifyAllowedEnergyTransfer(const ocpp::messages::ocpp21::NotifyAllowedEnergyTransferReq& request,
                                     ocpp::messages::ocpp21::NotifyAllowedEnergyTransferConf&      response,
                                     std::string&                                                  error,
                                     std::string&                                                  message) override;
    bool notifyChargingLimit(const ocpp::messages::ocpp21::NotifyChargingLimitReq& request,
                             ocpp::messages::ocpp21::NotifyChargingLimitConf&      response,
                             std::string&                                          error,
                             std::string&                                          message) override;
    bool notifyCustomerInformation(const ocpp::messages::ocpp21::NotifyCustomerInformationReq& request,
                                   ocpp::messages::ocpp21::NotifyCustomerInformationConf&      response,
                                   std::string&                                                error,
                                   std::string&                                                message) override;
    bool notifyDERAlarm(const ocpp::messages::ocpp21::NotifyDERAlarmReq& request,
                        ocpp::messages::ocpp21::NotifyDERAlarmConf&      response,
                        std::string&                                     error,
                        std::string&                                     message) override;
    bool notifyDERStartStop(const ocpp::messages::ocpp21::NotifyDERStartStopReq& request,
                            ocpp::messages::ocpp21::NotifyDERStartStopConf&      response,
                            std::string&                                         error,
                            std::string&                                         message) override;
    bool notifyDisplayMessages(const ocpp::messages::ocpp21::NotifyDisplayMessagesReq& request,
                               ocpp::messages::ocpp21::NotifyDisplayMessagesConf&      response,
                               std::string&                                            error,
                               std::string&                                            message) override;
    bool notifyEVChargingNeeds(const ocpp::messages::ocpp21::NotifyEVChargingNeedsReq& request,
                               ocpp::messages::ocpp21::NotifyEVChargingNeedsConf&      response,
                               std::string&                                            error,
                               std::string&                                            message) override;
    bool notifyEVChargingSchedule(const ocpp::messages::ocpp21::NotifyEVChargingScheduleReq& request,
                                  ocpp::messages::ocpp21::NotifyEVChargingScheduleConf&      response,
                                  std::string&                                               error,
                                  std::string&                                               message) override;
    bool notifyEvent(const ocpp::messages::ocpp21::NotifyEventReq& request,
                     ocpp::messages::ocpp21::NotifyEventConf&      response,
                     std::string&                                  error,
                     std::string&                                  message) override;
    bool notifyMonitoringReport(const ocpp::messages::ocpp21::NotifyMonitoringReportReq& request,
                                ocpp::messages::ocpp21::NotifyMonitoringReportConf&      response,
                                std::string&                                             error,
                                std::string&                                             message) override;
    bool notifyPeriodicEventStream(const ocpp::messages::ocpp21::NotifyPeriodicEventStreamReq& request,
                                   ocpp::messages::ocpp21::NotifyPeriodicEventStreamConf&      response,
                                   std::string&                                                error,
                                   std::string&                                                message) override;
    bool notifyPriorityCharging(const ocpp::messages::ocpp21::NotifyPriorityChargingReq& request,
                                ocpp::messages::ocpp21::NotifyPriorityChargingConf&      response,
                                std::string&                                             error,
                                std::string&                                             message) override;
    bool notifyReport(const ocpp::messages::ocpp21::NotifyReportReq& request,
                      ocpp::messages::ocpp21::NotifyReportConf&      response,
                      std::string&                                   error,
                      std::string&                                   message) override;
    bool notifySettlement(const ocpp::messages::ocpp21::NotifySettlementReq& request,
                          ocpp::messages::ocpp21::NotifySettlementConf&      response,
                          std::string&                                       error,
                          std::string&                                       message) override;
    bool notifyWebPaymentStarted(const ocpp::messages::ocpp21::NotifyWebPaymentStartedReq& request,
                                 ocpp::messages::ocpp21::NotifyWebPaymentStartedConf&      response,
                                 std::string&                                              error,
                                 std::string&                                              message) override;

  private:
    ocpp::messages::GenericMessageSender& m_msg_sender;

    template <typename RequestType, typename ResponseType>
    bool call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message);
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_NOTIFYMANAGER21_H
