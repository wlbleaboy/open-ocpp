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

#include "ProfileDatabase20.h"

#include "ChargingLimitSourceEnumType20.h"
#include "ChargingProfilePurposeEnumType20.h"
#include "Logger.h"

#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
ProfileDatabase20::ProfileDatabase20(ocpp::database::Database& database)
    : m_database(database), m_delete_query(), m_insert_query(), m_profiles()
{
    initDatabaseTable();
    load();
}

/** @brief Destructor */
ProfileDatabase20::~ProfileDatabase20() { }

/** @brief Clear one or multiple charging profiles with match criteria */
bool ProfileDatabase20::clear(Optional<int> id, Optional<ClearChargingProfileType> criteria)
{
    std::vector<int> profiles_to_delete;

    for (const ChargingProfileInfo& info : m_profiles)
    {
        if (matchClearCriteria(info, id, criteria))
        {
            profiles_to_delete.push_back(info.profile.id);
        }
    }

    for (int profile_id : profiles_to_delete)
    {
        deleteProfile(profile_id);
    }

    m_profiles.erase(std::remove_if(m_profiles.begin(),
                                    m_profiles.end(),
                                    [&profiles_to_delete](const ChargingProfileInfo& info) {
                                        return std::find(profiles_to_delete.begin(), profiles_to_delete.end(), info.profile.id) !=
                                               profiles_to_delete.end();
                                    }),
                     m_profiles.end());

    return !profiles_to_delete.empty();
}

/** @brief Clear TxProfile charging profiles for an EVSE */
bool ProfileDatabase20::clearTxProfiles(unsigned int evse_id)
{
    ClearChargingProfileType criteria;
    criteria.evseId                  = static_cast<int>(evse_id);
    criteria.chargingProfilePurpose  = ChargingProfilePurposeEnumType::TxProfile;
    Optional<ClearChargingProfileType> optional_criteria(criteria);
    return clear(Optional<int>(), optional_criteria);
}

/** @brief Install a charging profile */
bool ProfileDatabase20::install(unsigned int evse_id, const ChargingProfileType& profile)
{
    bool ret = false;

    if (!profile.chargingSchedule.empty())
    {
        for (const ChargingScheduleType& schedule : profile.chargingSchedule)
        {
            if (!schedule.chargingSchedulePeriod.empty())
            {
                ret = true;
                break;
            }
        }
    }

    if (ret)
    {
        std::vector<int> profiles_to_delete;
        for (const ChargingProfileInfo& installed : m_profiles)
        {
            if ((installed.profile.id == profile.id) ||
                ((installed.evse_id == evse_id) && (installed.profile.stackLevel == profile.stackLevel) &&
                 (installed.profile.chargingProfilePurpose == profile.chargingProfilePurpose)))
            {
                profiles_to_delete.push_back(installed.profile.id);
            }
        }

        for (int profile_id : profiles_to_delete)
        {
            deleteProfile(profile_id);
        }

        m_profiles.erase(std::remove_if(m_profiles.begin(),
                                        m_profiles.end(),
                                        [&profiles_to_delete](const ChargingProfileInfo& info) {
                                            return std::find(profiles_to_delete.begin(), profiles_to_delete.end(), info.profile.id) !=
                                                   profiles_to_delete.end();
                                        }),
                         m_profiles.end());

        m_profiles.push_back({evse_id, profile});
        insertProfile(evse_id, profile);
    }

    return ret;
}

/** @brief Assign the pending TxProfile of an EVSE to a transaction */
void ProfileDatabase20::assignPendingTxProfiles(unsigned int evse_id, const std::string& transaction_id)
{
    std::vector<int>            station_profiles_to_remove;
    std::vector<ChargingProfileInfo> profiles_to_assign;

    for (const ChargingProfileInfo& info : m_profiles)
    {
        if (((info.evse_id == 0u) || (info.evse_id == evse_id)) &&
            (info.profile.chargingProfilePurpose == ChargingProfilePurposeEnumType::TxProfile) && !info.profile.transactionId.isSet())
        {
            profiles_to_assign.push_back(info);
            if (info.evse_id == 0u)
            {
                station_profiles_to_remove.push_back(info.profile.id);
            }
        }
    }

    for (const ChargingProfileInfo& info : profiles_to_assign)
    {
        ChargingProfileType assigned_profile = info.profile;
        CiStringType<36u>   assigned_transaction;
        assigned_transaction.assign(transaction_id);
        assigned_profile.transactionId = assigned_transaction;
        install(evse_id, assigned_profile);
    }

    for (int profile_id : station_profiles_to_remove)
    {
        clear(profile_id);
    }
}

/** @brief Get profiles matching GetChargingProfiles criteria */
std::vector<ProfileDatabase20::ChargingProfileInfo> ProfileDatabase20::getProfiles(
    Optional<int> evse_id, const ChargingProfileCriterionType& criteria) const
{
    std::vector<ChargingProfileInfo> ret;

    for (const ChargingProfileInfo& info : m_profiles)
    {
        if (matchGetCriteria(info, evse_id, criteria))
        {
            ret.push_back(info);
        }
    }

    std::sort(ret.begin(), ret.end(), [](const ChargingProfileInfo& lhs, const ChargingProfileInfo& rhs) {
        if (lhs.evse_id == rhs.evse_id)
        {
            return lhs.profile.stackLevel > rhs.profile.stackLevel;
        }
        return lhs.evse_id < rhs.evse_id;
    });

    return ret;
}

/** @brief Get active profiles for an EVSE and the charging station */
std::vector<ProfileDatabase20::ChargingProfileInfo> ProfileDatabase20::getActiveProfiles(unsigned int evse_id) const
{
    std::vector<ChargingProfileInfo> ret;

    for (const ChargingProfileInfo& info : m_profiles)
    {
        if (((info.evse_id == 0u) || (info.evse_id == evse_id)) && isProfileValid(info.profile))
        {
            ret.push_back(info);
        }
    }

    std::sort(ret.begin(), ret.end(), [](const ChargingProfileInfo& lhs, const ChargingProfileInfo& rhs) {
        if (lhs.profile.stackLevel == rhs.profile.stackLevel)
        {
            return lhs.evse_id > rhs.evse_id;
        }
        return lhs.profile.stackLevel > rhs.profile.stackLevel;
    });

    return ret;
}

/** @brief Remove expired profiles */
bool ProfileDatabase20::cleanupExpiredProfiles()
{
    std::vector<int> profiles_to_delete;

    for (const ChargingProfileInfo& info : m_profiles)
    {
        if (!isProfileValid(info.profile))
        {
            profiles_to_delete.push_back(info.profile.id);
        }
    }

    for (int profile_id : profiles_to_delete)
    {
        deleteProfile(profile_id);
    }

    m_profiles.erase(std::remove_if(m_profiles.begin(),
                                    m_profiles.end(),
                                    [&profiles_to_delete](const ChargingProfileInfo& info) {
                                        return std::find(profiles_to_delete.begin(), profiles_to_delete.end(), info.profile.id) !=
                                               profiles_to_delete.end();
                                    }),
                     m_profiles.end());

    return !profiles_to_delete.empty();
}

/** @brief Initialize the database table */
void ProfileDatabase20::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS ChargingProfiles20 ("
                                  "[profile_id] INTEGER,"
                                  "[evse] INTEGER,"
                                  "[stack_level] INTEGER,"
                                  "[purpose] VARCHAR(64),"
                                  "[profile] VARCHAR(4096),"
                                  "PRIMARY KEY([profile_id]));");
    if (query)
    {
        query->exec();
    }

    m_delete_query = m_database.query("DELETE FROM ChargingProfiles20 WHERE profile_id=?;");
    m_insert_query = m_database.query("INSERT OR REPLACE INTO ChargingProfiles20 VALUES (?, ?, ?, ?, ?);");
}

/** @brief Load profiles from the database */
void ProfileDatabase20::load()
{
    auto query = m_database.query("SELECT profile_id, evse, profile FROM ChargingProfiles20 WHERE TRUE;");
    if (query && query->exec() && query->hasRows())
    {
        do
        {
            int          profile_id  = query->getInt32(0);
            unsigned int evse_id     = query->getUInt32(1);
            std::string  profile_str = query->getString(2);

            ChargingProfileType profile;
            if (deserialize(profile_str, profile) && (profile.id == profile_id))
            {
                m_profiles.push_back({evse_id, profile});
            }
            else
            {
                LOG_WARNING << "Invalid OCPP2 smart charging profile ignored : id = " << profile_id;
            }
        } while (query->next());
    }
}

/** @brief Remove a profile from the database */
void ProfileDatabase20::deleteProfile(int profile_id)
{
    if (m_delete_query)
    {
        m_delete_query->bind(0, profile_id);
        m_delete_query->exec();
        m_delete_query->reset();
    }
}

/** @brief Insert a profile in the database */
void ProfileDatabase20::insertProfile(unsigned int evse_id, const ChargingProfileType& profile)
{
    if (m_insert_query)
    {
        m_insert_query->bind(0, profile.id);
        m_insert_query->bind(1, evse_id);
        m_insert_query->bind(2, profile.stackLevel);
        m_insert_query->bind(3, ChargingProfilePurposeEnumTypeHelper.toString(profile.chargingProfilePurpose));
        m_insert_query->bind(4, serialize(profile));
        m_insert_query->exec();
        m_insert_query->reset();
    }
}

/** @brief Serialize a profile to a string */
std::string ProfileDatabase20::serialize(const ChargingProfileType& profile)
{
    rapidjson::Document        profile_json(rapidjson::kObjectType);
    ChargingProfileTypeConverter charging_profile_converter;
    charging_profile_converter.setAllocator(&profile_json.GetAllocator());
    charging_profile_converter.toJson(profile, profile_json);

    rapidjson::StringBuffer                    buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.SetMaxDecimalPlaces(1);
    profile_json.Accept(writer);
    return buffer.GetString();
}

/** @brief Deserialize a profile from a string */
bool ProfileDatabase20::deserialize(const std::string& profile_str, ChargingProfileType& profile)
{
    std::string         error_code;
    std::string         error_message;
    rapidjson::Document profile_json;
    profile_json.Parse(profile_str.c_str());

    ChargingProfileTypeConverter charging_profile_converter;
    charging_profile_converter.setAllocator(&profile_json.GetAllocator());
    return charging_profile_converter.fromJson(profile_json, profile, error_code, error_message);
}

/** @brief Check ClearChargingProfile criteria */
bool ProfileDatabase20::matchClearCriteria(const ChargingProfileInfo& info,
                                           Optional<int>              id,
                                           Optional<ClearChargingProfileType> criteria) const
{
    bool ret = true;

    if (id.isSet())
    {
        ret = (info.profile.id == id.value());
    }
    else if (criteria.isSet())
    {
        const ClearChargingProfileType& clear_criteria = criteria.value();
        ret = ((!clear_criteria.evseId.isSet()) || (static_cast<int>(info.evse_id) == clear_criteria.evseId.value()));
        ret = ret && ((!clear_criteria.chargingProfilePurpose.isSet()) ||
                      (info.profile.chargingProfilePurpose == clear_criteria.chargingProfilePurpose.value()));
        ret = ret && ((!clear_criteria.stackLevel.isSet()) || (info.profile.stackLevel == clear_criteria.stackLevel.value()));
    }

    return ret;
}

/** @brief Check GetChargingProfiles criteria */
bool ProfileDatabase20::matchGetCriteria(const ChargingProfileInfo& info,
                                         Optional<int>              evse_id,
                                         const ChargingProfileCriterionType& criteria) const
{
    bool ret = ((!evse_id.isSet()) || (static_cast<int>(info.evse_id) == evse_id.value()));

    ret = ret && ((!criteria.chargingProfilePurpose.isSet()) ||
                  (info.profile.chargingProfilePurpose == criteria.chargingProfilePurpose.value()));
    ret = ret && ((!criteria.stackLevel.isSet()) || (info.profile.stackLevel == criteria.stackLevel.value()));

    if (ret && !criteria.chargingProfileId.empty())
    {
        ret = (std::find(criteria.chargingProfileId.begin(), criteria.chargingProfileId.end(), info.profile.id) !=
               criteria.chargingProfileId.end());
    }

    if (ret && !criteria.chargingLimitSource.empty())
    {
        ret = (std::find(criteria.chargingLimitSource.begin(),
                         criteria.chargingLimitSource.end(),
                         ChargingLimitSourceEnumType::CSO) != criteria.chargingLimitSource.end());
    }

    return ret;
}

/** @brief Check if a profile is currently valid */
bool ProfileDatabase20::isProfileValid(const ChargingProfileType& profile) const
{
    bool ret = true;
    DateTime now = DateTime::now();

    if (profile.validFrom.isSet())
    {
        ret = (profile.validFrom.value() <= now);
    }

    if (ret && profile.validTo.isSet())
    {
        ret = (profile.validTo.value() >= now);
    }

    return ret;
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
