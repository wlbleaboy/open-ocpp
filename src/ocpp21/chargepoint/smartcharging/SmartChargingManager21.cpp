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

#include "SmartChargingManager21.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

SmartChargingManager21::SmartChargingManager21(IChargePointEventsHandler21&                    events_handler,
             const ocpp::messages::GenericMessagesConverter& messages_converter,
             ocpp::messages::IMessageDispatcher&             msg_dispatcher,
             ocpp::messages::GenericMessageSender&           msg_sender)
    : GenericMessageHandler<AFRRSignalReq, AFRRSignalConf>(AFRRSIGNAL_ACTION, messages_converter),
      GenericMessageHandler<ClearChargingProfileReq, ClearChargingProfileConf>(CLEARCHARGINGPROFILE_ACTION, messages_converter),
      GenericMessageHandler<ClearDERControlReq, ClearDERControlConf>(CLEARDERCONTROL_ACTION, messages_converter),
      GenericMessageHandler<GetChargingProfilesReq, GetChargingProfilesConf>(GETCHARGINGPROFILES_ACTION, messages_converter),
      GenericMessageHandler<GetCompositeScheduleReq, GetCompositeScheduleConf>(GETCOMPOSITESCHEDULE_ACTION, messages_converter),
      GenericMessageHandler<GetDERControlReq, GetDERControlConf>(GETDERCONTROL_ACTION, messages_converter),
      GenericMessageHandler<SetChargingProfileReq, SetChargingProfileConf>(SETCHARGINGPROFILE_ACTION, messages_converter),
      GenericMessageHandler<SetDERControlReq, SetDERControlConf>(SETDERCONTROL_ACTION, messages_converter),
      GenericMessageHandler<SetDefaultTariffReq, SetDefaultTariffConf>(SETDEFAULTTARIFF_ACTION, messages_converter),
      GenericMessageHandler<UpdateDynamicScheduleReq, UpdateDynamicScheduleConf>(UPDATEDYNAMICSCHEDULE_ACTION, messages_converter),
      GenericMessageHandler<UsePriorityChargingReq, UsePriorityChargingConf>(USEPRIORITYCHARGING_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender)
{
    msg_dispatcher.registerHandler(AFRRSIGNAL_ACTION, *dynamic_cast<GenericMessageHandler<AFRRSignalReq, AFRRSignalConf>*>(this));
    msg_dispatcher.registerHandler(CLEARCHARGINGPROFILE_ACTION, *dynamic_cast<GenericMessageHandler<ClearChargingProfileReq, ClearChargingProfileConf>*>(this));
    msg_dispatcher.registerHandler(CLEARDERCONTROL_ACTION, *dynamic_cast<GenericMessageHandler<ClearDERControlReq, ClearDERControlConf>*>(this));
    msg_dispatcher.registerHandler(GETCHARGINGPROFILES_ACTION, *dynamic_cast<GenericMessageHandler<GetChargingProfilesReq, GetChargingProfilesConf>*>(this));
    msg_dispatcher.registerHandler(GETCOMPOSITESCHEDULE_ACTION, *dynamic_cast<GenericMessageHandler<GetCompositeScheduleReq, GetCompositeScheduleConf>*>(this));
    msg_dispatcher.registerHandler(GETDERCONTROL_ACTION, *dynamic_cast<GenericMessageHandler<GetDERControlReq, GetDERControlConf>*>(this));
    msg_dispatcher.registerHandler(SETCHARGINGPROFILE_ACTION, *dynamic_cast<GenericMessageHandler<SetChargingProfileReq, SetChargingProfileConf>*>(this));
    msg_dispatcher.registerHandler(SETDERCONTROL_ACTION, *dynamic_cast<GenericMessageHandler<SetDERControlReq, SetDERControlConf>*>(this));
    msg_dispatcher.registerHandler(SETDEFAULTTARIFF_ACTION, *dynamic_cast<GenericMessageHandler<SetDefaultTariffReq, SetDefaultTariffConf>*>(this));
    msg_dispatcher.registerHandler(UPDATEDYNAMICSCHEDULE_ACTION, *dynamic_cast<GenericMessageHandler<UpdateDynamicScheduleReq, UpdateDynamicScheduleConf>*>(this));
    msg_dispatcher.registerHandler(USEPRIORITYCHARGING_ACTION, *dynamic_cast<GenericMessageHandler<UsePriorityChargingReq, UsePriorityChargingConf>*>(this));
}

SmartChargingManager21::~SmartChargingManager21()
{
}

bool SmartChargingManager21::call(const ClearedChargingLimitReq& request, ClearedChargingLimitConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(CLEAREDCHARGINGLIMIT_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyAllowedEnergyTransferReq& request, NotifyAllowedEnergyTransferConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYALLOWEDENERGYTRANSFER_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyChargingLimitReq& request, NotifyChargingLimitConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYCHARGINGLIMIT_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyDERAlarmReq& request, NotifyDERAlarmConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYDERALARM_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyDERStartStopReq& request, NotifyDERStartStopConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYDERSTARTSTOP_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyEVChargingNeedsReq& request, NotifyEVChargingNeedsConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYEVCHARGINGNEEDS_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyEVChargingScheduleReq& request, NotifyEVChargingScheduleConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYEVCHARGINGSCHEDULE_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const NotifyPriorityChargingReq& request, NotifyPriorityChargingConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(NOTIFYPRIORITYCHARGING_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const PullDynamicScheduleUpdateReq& request, PullDynamicScheduleUpdateConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(PULLDYNAMICSCHEDULEUPDATE_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const ReportChargingProfilesReq& request, ReportChargingProfilesConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(REPORTCHARGINGPROFILES_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::call(const ReportDERControlReq& request, ReportDERControlConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(REPORTDERCONTROL_ACTION, request, response, error, message) == CallResult::Ok);
}

bool SmartChargingManager21::getSetpoint(unsigned int                                  evse_id,
                                         ocpp::types::Optional<SmartChargingSetpoint>& charging_station_setpoint,
                                         ocpp::types::Optional<SmartChargingSetpoint>& evse_setpoint,
                                         ocpp::types::ocpp21::ChargingRateUnitEnumType unit)
{
    (void)evse_id;
    (void)unit;

    charging_station_setpoint.clear();
    evse_setpoint.clear();
    return false;
}

bool SmartChargingManager21::handleMessage(const AFRRSignalReq& request,
                               AFRRSignalConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onAFRRSignal(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const ClearChargingProfileReq& request,
                               ClearChargingProfileConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onClearChargingProfile(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const ClearDERControlReq& request,
                               ClearDERControlConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onClearDERControl(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const GetChargingProfilesReq& request,
                               GetChargingProfilesConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onGetChargingProfiles(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const GetCompositeScheduleReq& request,
                               GetCompositeScheduleConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onGetCompositeSchedule(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const GetDERControlReq& request,
                               GetDERControlConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onGetDERControl(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const SetChargingProfileReq& request,
                               SetChargingProfileConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onSetChargingProfile(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const SetDERControlReq& request,
                               SetDERControlConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onSetDERControl(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const SetDefaultTariffReq& request,
                               SetDefaultTariffConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onSetDefaultTariff(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const UpdateDynamicScheduleReq& request,
                               UpdateDynamicScheduleConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onUpdateDynamicSchedule(request, response, error_code, error_message);
}

bool SmartChargingManager21::handleMessage(const UsePriorityChargingReq& request,
                               UsePriorityChargingConf&      response,
                               std::string&    error_code,
                               std::string&    error_message)
{
    return m_events_handler.onUsePriorityCharging(request, response, error_code, error_message);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
