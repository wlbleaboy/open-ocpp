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

#ifndef OPENOCPP_OCPP20_NOTIFYMANAGER20_H
#define OPENOCPP_OCPP20_NOTIFYMANAGER20_H

#include "INotifyManager20.h"

#include <string>

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
} // namespace messages

namespace chargepoint
{
namespace ocpp20
{

/** @brief Handle OCPP 2.0.1 Notify* outbound messages */
class NotifyManager20 : public INotifyManager20
{
  public:
    /** @brief Constructor */
    NotifyManager20(ocpp::messages::GenericMessageSender& msg_sender);

    /** @brief Destructor */
    virtual ~NotifyManager20();

    /** @brief Send a NotifyChargingLimit message */
    bool notifyChargingLimit(const ocpp::messages::ocpp20::NotifyChargingLimitReq& request,
                             ocpp::messages::ocpp20::NotifyChargingLimitConf&      response,
                             std::string&                                          error,
                             std::string&                                          message) override;

    /** @brief Send a NotifyCustomerInformation message */
    bool notifyCustomerInformation(const ocpp::messages::ocpp20::NotifyCustomerInformationReq& request,
                                   ocpp::messages::ocpp20::NotifyCustomerInformationConf&      response,
                                   std::string&                                                error,
                                   std::string&                                                message) override;

    /** @brief Send a NotifyDisplayMessages message */
    bool notifyDisplayMessages(const ocpp::messages::ocpp20::NotifyDisplayMessagesReq& request,
                               ocpp::messages::ocpp20::NotifyDisplayMessagesConf&      response,
                               std::string&                                            error,
                               std::string&                                            message) override;

    /** @brief Send a NotifyEVChargingNeeds message */
    bool notifyEVChargingNeeds(const ocpp::messages::ocpp20::NotifyEVChargingNeedsReq& request,
                               ocpp::messages::ocpp20::NotifyEVChargingNeedsConf&      response,
                               std::string&                                            error,
                               std::string&                                            message) override;

    /** @brief Send a NotifyEVChargingSchedule message */
    bool notifyEVChargingSchedule(const ocpp::messages::ocpp20::NotifyEVChargingScheduleReq& request,
                                  ocpp::messages::ocpp20::NotifyEVChargingScheduleConf&      response,
                                  std::string&                                               error,
                                  std::string&                                               message) override;

    /** @brief Send a NotifyEvent message */
    bool notifyEvent(const ocpp::messages::ocpp20::NotifyEventReq& request,
                     ocpp::messages::ocpp20::NotifyEventConf&      response,
                     std::string&                                  error,
                     std::string&                                  message) override;

    /** @brief Send a NotifyMonitoringReport message */
    bool notifyMonitoringReport(const ocpp::messages::ocpp20::NotifyMonitoringReportReq& request,
                                ocpp::messages::ocpp20::NotifyMonitoringReportConf&      response,
                                std::string&                                             error,
                                std::string&                                             message) override;

    /** @brief Send a NotifyReport message */
    bool notifyReport(const ocpp::messages::ocpp20::NotifyReportReq& request,
                      ocpp::messages::ocpp20::NotifyReportConf&      response,
                      std::string&                                   error,
                      std::string&                                   message) override;

  private:
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;

    /** @brief Execute a notify call */
    template <typename RequestType, typename ResponseType>
    bool call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message);
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_NOTIFYMANAGER20_H
