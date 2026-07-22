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

#include "SmartChargingManager20.h"

#include "GenericMessageSender.h"
#include "IChargePointEventsHandler20.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "ReportChargingProfiles20.h"

#include <algorithm>
#include <iterator>
#include <map>

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;
using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
SmartChargingManager20::SmartChargingManager20(ocpp::database::Database&                       database,
                                               IChargePointEventsHandler20&                    events_handler,
                                               const ocpp::messages::GenericMessagesConverter& messages_converter,
                                               ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                               ocpp::messages::GenericMessageSender&           msg_sender)
    : GenericMessageHandler<ClearChargingProfileReq, ClearChargingProfileConf>(CLEARCHARGINGPROFILE_ACTION, messages_converter),
      GenericMessageHandler<GetChargingProfilesReq, GetChargingProfilesConf>(GETCHARGINGPROFILES_ACTION, messages_converter),
      GenericMessageHandler<GetCompositeScheduleReq, GetCompositeScheduleConf>(GETCOMPOSITESCHEDULE_ACTION, messages_converter),
      GenericMessageHandler<SetChargingProfileReq, SetChargingProfileConf>(SETCHARGINGPROFILE_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_profile_db(database),
      m_mutex()
{
    msg_dispatcher.registerHandler(
        CLEARCHARGINGPROFILE_ACTION,
        *dynamic_cast<GenericMessageHandler<ClearChargingProfileReq, ClearChargingProfileConf>*>(this));
    msg_dispatcher.registerHandler(GETCHARGINGPROFILES_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetChargingProfilesReq, GetChargingProfilesConf>*>(this));
    msg_dispatcher.registerHandler(
        GETCOMPOSITESCHEDULE_ACTION,
        *dynamic_cast<GenericMessageHandler<GetCompositeScheduleReq, GetCompositeScheduleConf>*>(this));
    msg_dispatcher.registerHandler(SETCHARGINGPROFILE_ACTION,
                                   *dynamic_cast<GenericMessageHandler<SetChargingProfileReq, SetChargingProfileConf>*>(this));
}

/** @brief Destructor */
SmartChargingManager20::~SmartChargingManager20() { }

/** @copydoc bool ISmartChargingManager20::clearedChargingLimit(...) */
bool SmartChargingManager20::clearedChargingLimit(const ClearedChargingLimitReq& request,
                                                  ClearedChargingLimitConf&      response,
                                                  std::string&                   error,
                                                  std::string&                   message)
{
    bool ret = false;

    CallResult result = m_msg_sender.call(CLEAREDCHARGINGLIMIT_ACTION, request, response, error, message);
    if (result == CallResult::Ok)
    {
        ret = true;
    }
    else
    {
        LOG_ERROR << "ClearedChargingLimit => " << (result == CallResult::Failed ? "Timeout" : "Error");
    }

    return ret;
}

/** @brief Handle ClearChargingProfile */
bool SmartChargingManager20::handleMessage(const ClearChargingProfileReq& request,
                                           ClearChargingProfileConf&      response,
                                           std::string&                   error_code,
                                           std::string&                   error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "ClearChargingProfile request received : chargingProfileId = "
             << (request.chargingProfileId.isSet() ? std::to_string(request.chargingProfileId.value()) : "not set");

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool cleared = m_profile_db.clear(request.chargingProfileId, request.chargingProfileCriteria);
        response.status = cleared ? ClearChargingProfileStatusEnumType::Accepted : ClearChargingProfileStatusEnumType::Unknown;
    }

    return true;
}

/** @brief Handle GetChargingProfiles */
bool SmartChargingManager20::handleMessage(const GetChargingProfilesReq& request,
                                           GetChargingProfilesConf&      response,
                                           std::string&                  error_code,
                                           std::string&                  error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "GetChargingProfiles request received : requestId = " << request.requestId
             << " - evseId = " << (request.evseId.isSet() ? std::to_string(request.evseId.value()) : "not set");

    std::vector<ProfileDatabase20::ChargingProfileInfo> profiles;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_profile_db.cleanupExpiredProfiles();
        profiles = m_profile_db.getProfiles(request.evseId, request.chargingProfile);
    }

    if (!profiles.empty())
    {
        response.status = GetChargingProfileStatusEnumType::Accepted;
        reportChargingProfiles(request.requestId, profiles);
    }
    else
    {
        response.status = GetChargingProfileStatusEnumType::NoProfiles;
    }

    return true;
}

/** @brief Handle GetCompositeSchedule */
bool SmartChargingManager20::handleMessage(const GetCompositeScheduleReq& request,
                                           GetCompositeScheduleConf&      response,
                                           std::string&                   error_code,
                                           std::string&                   error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "GetCompositeSchedule request received : evseId = " << request.evseId << " - duration = " << request.duration;

    CompositeScheduleType schedule;
    bool                  accepted = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_profile_db.cleanupExpiredProfiles();
        accepted = buildCompositeSchedule(request.evseId, request.duration, request.chargingRateUnit, schedule);
    }

    if (accepted)
    {
        response.status   = GenericStatusEnumType::Accepted;
        response.schedule = schedule;
    }
    else
    {
        response.status = GenericStatusEnumType::Rejected;
    }

    return true;
}

/** @brief Handle SetChargingProfile */
bool SmartChargingManager20::handleMessage(const SetChargingProfileReq& request,
                                           SetChargingProfileConf&      response,
                                           std::string&                 error_code,
                                           std::string&                 error_message)
{
    (void)error_code;
    (void)error_message;

    LOG_INFO << "SetChargingProfile request received : evseId = " << request.evseId
             << " - chargingProfileId = " << request.chargingProfile.id;

    bool installed = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        installed = isProfileValid(request.chargingProfile) &&
                    m_profile_db.install(static_cast<unsigned int>(request.evseId), request.chargingProfile);
    }

    response.status = installed ? ChargingProfileStatusEnumType::Accepted : ChargingProfileStatusEnumType::Rejected;
    return true;
}

/** @copydoc bool ISmartChargingManager20::getSetpoint(...) */
bool SmartChargingManager20::getSetpoint(unsigned int                                  evse_id,
                                         Optional<SmartChargingSetpoint>&              charging_station_setpoint,
                                         Optional<SmartChargingSetpoint>&              evse_setpoint,
                                         ChargingRateUnitEnumType                      unit)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_profile_db.cleanupExpiredProfiles();

    SmartChargingSetpoint station_setpoint;
    bool                  station_set = getActiveSetpoint(evse_id, true, unit, station_setpoint);
    if (station_set)
    {
        charging_station_setpoint = station_setpoint;
    }
    else
    {
        charging_station_setpoint.clear();
    }

    SmartChargingSetpoint selected_evse_setpoint;
    bool                  evse_set = getActiveSetpoint(evse_id, false, unit, selected_evse_setpoint);
    if (evse_set)
    {
        evse_setpoint = selected_evse_setpoint;
    }
    else
    {
        evse_setpoint.clear();
    }

    return (station_set || evse_set);
}

/** @copydoc bool ISmartChargingManager20::installTxProfile(...) */
bool SmartChargingManager20::installTxProfile(unsigned int evse_id, const ChargingProfileType& profile)
{
    bool ret = false;
    if (profile.chargingProfilePurpose == ChargingProfilePurposeEnumType::TxProfile)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ret = isProfileValid(profile) && m_profile_db.install(evse_id, profile);
    }
    return ret;
}

/** @copydoc void ISmartChargingManager20::assignPendingTxProfiles(...) */
void SmartChargingManager20::assignPendingTxProfiles(unsigned int evse_id, const std::string& transaction_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profile_db.assignPendingTxProfiles(evse_id, transaction_id);
}

/** @copydoc void ISmartChargingManager20::clearTxProfiles(...) */
void SmartChargingManager20::clearTxProfiles(unsigned int evse_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profile_db.clearTxProfiles(evse_id);
}

/** @brief Send a ReportChargingProfiles request */
bool SmartChargingManager20::reportChargingProfiles(int request_id, const std::vector<ProfileDatabase20::ChargingProfileInfo>& profiles)
{
    bool ret = true;

    std::map<unsigned int, std::vector<ChargingProfileType>> profiles_by_evse;
    for (const ProfileDatabase20::ChargingProfileInfo& info : profiles)
    {
        profiles_by_evse[info.evse_id].push_back(info.profile);
    }

    for (auto iter = profiles_by_evse.begin(); iter != profiles_by_evse.end(); ++iter)
    {
        ReportChargingProfilesReq request;
        request.requestId            = request_id;
        request.chargingLimitSource  = ChargingLimitSourceEnumType::CSO;
        request.chargingProfile      = iter->second;
        request.evseId               = static_cast<int>(iter->first);
        request.tbc                  = (std::next(iter) != profiles_by_evse.end());

        ReportChargingProfilesConf response;
        std::string                 error;
        std::string                 message;
        CallResult result = m_msg_sender.call(REPORTCHARGINGPROFILES_ACTION, request, response, error, message);
        if ((result != CallResult::Ok) && (result != CallResult::Delayed))
        {
            LOG_ERROR << "Unable to report OCPP2 charging profiles : result = " << CallResultHelper.toString(result)
                      << " - error = " << error << " - message = " << message;
            ret = false;
        }
    }

    return ret;
}

/** @brief Build a composite schedule from active profiles */
bool SmartChargingManager20::buildCompositeSchedule(int                                      evse_id,
                                                    int                                      duration,
                                                    Optional<ChargingRateUnitEnumType>       requested_unit,
                                                    CompositeScheduleType&                   schedule)
{
    bool ret = false;

    std::vector<ProfileDatabase20::ChargingProfileInfo> profiles = m_profile_db.getActiveProfiles(static_cast<unsigned int>(evse_id));
    for (const ProfileDatabase20::ChargingProfileInfo& info : profiles)
    {
        for (const ChargingScheduleType& charging_schedule : info.profile.chargingSchedule)
        {
            if (charging_schedule.chargingSchedulePeriod.empty())
            {
                continue;
            }

            if (requested_unit.isSet() && (charging_schedule.chargingRateUnit != requested_unit.value()))
            {
                continue;
            }

            schedule.evseId           = evse_id;
            schedule.duration         = duration;
            schedule.scheduleStart    = DateTime::now();
            schedule.chargingRateUnit = charging_schedule.chargingRateUnit;
            schedule.chargingSchedulePeriod.clear();

            for (const ChargingSchedulePeriodType& period : charging_schedule.chargingSchedulePeriod)
            {
                if ((duration <= 0) || (period.startPeriod < duration))
                {
                    schedule.chargingSchedulePeriod.push_back(period);
                }
            }

            ret = !schedule.chargingSchedulePeriod.empty();
            if (ret)
            {
                break;
            }
        }

        if (ret)
        {
            break;
        }
    }

    return ret;
}

/** @brief Extract the current setpoint from active profiles */
bool SmartChargingManager20::getActiveSetpoint(unsigned int             evse_id,
                                               bool                     station_profile,
                                               ChargingRateUnitEnumType unit,
                                               SmartChargingSetpoint&   setpoint)
{
    bool ret = false;

    std::vector<ProfileDatabase20::ChargingProfileInfo> profiles = m_profile_db.getActiveProfiles(evse_id);
    for (const ProfileDatabase20::ChargingProfileInfo& info : profiles)
    {
        if (station_profile != (info.evse_id == 0u))
        {
            continue;
        }

        for (const ChargingScheduleType& schedule : info.profile.chargingSchedule)
        {
            if ((schedule.chargingRateUnit != unit) || schedule.chargingSchedulePeriod.empty())
            {
                continue;
            }

            const ChargingSchedulePeriodType* current_period = nullptr;
            for (const ChargingSchedulePeriodType& period : schedule.chargingSchedulePeriod)
            {
                if (period.startPeriod <= 0)
                {
                    current_period = &period;
                }
            }

            if (!current_period)
            {
                current_period = &schedule.chargingSchedulePeriod.front();
            }

            setpoint.limit        = current_period->limit;
            setpoint.unit         = schedule.chargingRateUnit;
            setpoint.numberPhases = current_period->numberPhases;
            setpoint.phaseToUse   = current_period->phaseToUse;
            ret                   = true;
            break;
        }

        if (ret)
        {
            break;
        }
    }

    return ret;
}

/** @brief Check if a charging profile contains a usable schedule */
bool SmartChargingManager20::isProfileValid(const ChargingProfileType& profile) const
{
    bool ret = !profile.chargingSchedule.empty();

    for (const ChargingScheduleType& schedule : profile.chargingSchedule)
    {
        ret = ret && !schedule.chargingSchedulePeriod.empty();
    }

    return ret;
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
