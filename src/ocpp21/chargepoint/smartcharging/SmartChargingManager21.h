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

#ifndef OPENOCPP_OCPP21_SMARTCHARGINGMANAGER21_H
#define OPENOCPP_OCPP21_SMARTCHARGINGMANAGER21_H

#include "AFRRSignal21.h"
#include "ClearChargingProfile21.h"
#include "ClearDERControl21.h"
#include "ClearedChargingLimit21.h"
#include "ChargingRateUnitEnumType21.h"
#include "GetChargingProfiles21.h"
#include "GetCompositeSchedule21.h"
#include "GetDERControl21.h"
#include "NotifyAllowedEnergyTransfer21.h"
#include "NotifyChargingLimit21.h"
#include "NotifyDERAlarm21.h"
#include "NotifyDERStartStop21.h"
#include "NotifyEVChargingNeeds21.h"
#include "NotifyEVChargingSchedule21.h"
#include "NotifyPriorityCharging21.h"
#include "PullDynamicScheduleUpdate21.h"
#include "ReportChargingProfiles21.h"
#include "ReportDERControl21.h"
#include "SetChargingProfile21.h"
#include "SetDERControl21.h"
#include "SetDefaultTariff21.h"
#include "UpdateDynamicSchedule21.h"
#include "UsePriorityCharging21.h"
#include "GenericMessageHandler.h"
#include "Optional.h"

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 smart charging requests for the charge point */
class SmartChargingManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::AFRRSignalReq, ocpp::messages::ocpp21::AFRRSignalConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ClearChargingProfileReq, ocpp::messages::ocpp21::ClearChargingProfileConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ClearDERControlReq, ocpp::messages::ocpp21::ClearDERControlConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetChargingProfilesReq, ocpp::messages::ocpp21::GetChargingProfilesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetCompositeScheduleReq, ocpp::messages::ocpp21::GetCompositeScheduleConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::GetDERControlReq, ocpp::messages::ocpp21::GetDERControlConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetChargingProfileReq, ocpp::messages::ocpp21::SetChargingProfileConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetDERControlReq, ocpp::messages::ocpp21::SetDERControlConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SetDefaultTariffReq, ocpp::messages::ocpp21::SetDefaultTariffConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::UpdateDynamicScheduleReq, ocpp::messages::ocpp21::UpdateDynamicScheduleConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::UsePriorityChargingReq, ocpp::messages::ocpp21::UsePriorityChargingConf>
{
  public:
    /** @brief Smart charging setpoint */
    struct SmartChargingSetpoint
    {
        float value;
        ocpp::types::ocpp21::ChargingRateUnitEnumType unit;
    };

    SmartChargingManager21(IChargePointEventsHandler21&                         events_handler,
          const ocpp::messages::GenericMessagesConverter&      messages_converter,
          ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
          ocpp::messages::GenericMessageSender&                msg_sender);
    virtual ~SmartChargingManager21();

    bool call(const ocpp::messages::ocpp21::ClearedChargingLimitReq& request,
              ocpp::messages::ocpp21::ClearedChargingLimitConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyAllowedEnergyTransferReq& request,
              ocpp::messages::ocpp21::NotifyAllowedEnergyTransferConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyChargingLimitReq& request,
              ocpp::messages::ocpp21::NotifyChargingLimitConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyDERAlarmReq& request,
              ocpp::messages::ocpp21::NotifyDERAlarmConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyDERStartStopReq& request,
              ocpp::messages::ocpp21::NotifyDERStartStopConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyEVChargingNeedsReq& request,
              ocpp::messages::ocpp21::NotifyEVChargingNeedsConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyEVChargingScheduleReq& request,
              ocpp::messages::ocpp21::NotifyEVChargingScheduleConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::NotifyPriorityChargingReq& request,
              ocpp::messages::ocpp21::NotifyPriorityChargingConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::PullDynamicScheduleUpdateReq& request,
              ocpp::messages::ocpp21::PullDynamicScheduleUpdateConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::ReportChargingProfilesReq& request,
              ocpp::messages::ocpp21::ReportChargingProfilesConf&      response,
              std::string&                              error,
              std::string&                              message);
    bool call(const ocpp::messages::ocpp21::ReportDERControlReq& request,
              ocpp::messages::ocpp21::ReportDERControlConf&      response,
              std::string&                              error,
              std::string&                              message);

    bool getSetpoint(unsigned int                                               evse_id,
                     ocpp::types::Optional<SmartChargingSetpoint>&              charging_station_setpoint,
                     ocpp::types::Optional<SmartChargingSetpoint>&              evse_setpoint,
                     ocpp::types::ocpp21::ChargingRateUnitEnumType              unit);

    bool handleMessage(const ocpp::messages::ocpp21::AFRRSignalReq& request,
                       ocpp::messages::ocpp21::AFRRSignalConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::ClearChargingProfileReq& request,
                       ocpp::messages::ocpp21::ClearChargingProfileConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::ClearDERControlReq& request,
                       ocpp::messages::ocpp21::ClearDERControlConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetChargingProfilesReq& request,
                       ocpp::messages::ocpp21::GetChargingProfilesConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetCompositeScheduleReq& request,
                       ocpp::messages::ocpp21::GetCompositeScheduleConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::GetDERControlReq& request,
                       ocpp::messages::ocpp21::GetDERControlConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetChargingProfileReq& request,
                       ocpp::messages::ocpp21::SetChargingProfileConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetDERControlReq& request,
                       ocpp::messages::ocpp21::SetDERControlConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::SetDefaultTariffReq& request,
                       ocpp::messages::ocpp21::SetDefaultTariffConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::UpdateDynamicScheduleReq& request,
                       ocpp::messages::ocpp21::UpdateDynamicScheduleConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::UsePriorityChargingReq& request,
                       ocpp::messages::ocpp21::UsePriorityChargingConf&      response,
                       std::string&                           error_code,
                       std::string&                           error_message) override;

  private:
    IChargePointEventsHandler21& m_events_handler;
    ocpp::messages::GenericMessageSender& m_msg_sender;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_SMARTCHARGINGMANAGER21_H
