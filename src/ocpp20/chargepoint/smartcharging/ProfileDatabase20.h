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

#ifndef OPENOCPP_OCPP20_PROFILEDATABASE20_H
#define OPENOCPP_OCPP20_PROFILEDATABASE20_H

#include "ChargingProfileCriterionType20.h"
#include "ChargingProfileType20.h"
#include "ClearChargingProfileType20.h"
#include "Database.h"
#include "Optional.h"

#include <memory>
#include <vector>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Handle persistency of OCPP 2.0.1 charging profiles */
class ProfileDatabase20
{
  public:
    /** @brief Stored profile with its EVSE target */
    struct ChargingProfileInfo
    {
        /** @brief Target EVSE. 0 means charging station profile */
        unsigned int evse_id;
        /** @brief Charging profile */
        ocpp::types::ocpp20::ChargingProfileType profile;
    };

    /** @brief Constructor */
    ProfileDatabase20(ocpp::database::Database& database);

    /** @brief Destructor */
    virtual ~ProfileDatabase20();

    /**
     * @brief Clear one or multiple charging profiles with match criteria
     * @return true if at least one profile matched the criteria, false otherwise
     */
    bool clear(ocpp::types::Optional<int> id,
               ocpp::types::Optional<ocpp::types::ocpp20::ClearChargingProfileType> criteria =
                   ocpp::types::Optional<ocpp::types::ocpp20::ClearChargingProfileType>());

    /**
     * @brief Clear TxProfile charging profiles for an EVSE
     * @param evse_id Id of the EVSE
     * @return true if at least one profile matched the criteria, false otherwise
     */
    bool clearTxProfiles(unsigned int evse_id);

    /**
     * @brief Install a charging profile
     * @param evse_id Id of the EVSE targeted by the charging profile
     * @param profile Charging profile to install
     * @return true if the charging profile has been installed, false otherwise
     */
    bool install(unsigned int evse_id, const ocpp::types::ocpp20::ChargingProfileType& profile);

    /**
     * @brief Assign the pending TxProfile of an EVSE to a transaction
     * @param evse_id Id of the EVSE targeted by the charging profile
     * @param transaction_id Transaction to associate with the profile
     */
    void assignPendingTxProfiles(unsigned int evse_id, const std::string& transaction_id);

    /**
     * @brief Get profiles matching GetChargingProfiles criteria
     * @param evse_id Optional EVSE filter
     * @param criteria Charging profile criteria
     * @return Matching profiles
     */
    std::vector<ChargingProfileInfo> getProfiles(
        ocpp::types::Optional<int>                                   evse_id,
        const ocpp::types::ocpp20::ChargingProfileCriterionType& criteria) const;

    /**
     * @brief Get active profiles for an EVSE and the charging station
     * @param evse_id EVSE id
     * @return Matching profiles sorted by stack level, highest first
     */
    std::vector<ChargingProfileInfo> getActiveProfiles(unsigned int evse_id) const;

    /** @brief Remove expired profiles */
    bool cleanupExpiredProfiles();

  private:
    /** @brief Charge point's database */
    ocpp::database::Database& m_database;

    /** @brief Query to delete a profile */
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    /** @brief Query to insert a profile */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;

    /** @brief Installed charging profiles */
    std::vector<ChargingProfileInfo> m_profiles;

    /** @brief Initialize the database table */
    void initDatabaseTable();

    /** @brief Load profiles from the database */
    void load();

    /** @brief Remove a profile from the database */
    void deleteProfile(int profile_id);

    /** @brief Insert a profile in the database */
    void insertProfile(unsigned int evse_id, const ocpp::types::ocpp20::ChargingProfileType& profile);

    /** @brief Serialize a profile to a string */
    std::string serialize(const ocpp::types::ocpp20::ChargingProfileType& profile);

    /** @brief Deserialize a profile from a string */
    bool deserialize(const std::string& profile_str, ocpp::types::ocpp20::ChargingProfileType& profile);

    /** @brief Check ClearChargingProfile criteria */
    bool matchClearCriteria(const ChargingProfileInfo& info,
                            ocpp::types::Optional<int> id,
                            ocpp::types::Optional<ocpp::types::ocpp20::ClearChargingProfileType> criteria) const;

    /** @brief Check GetChargingProfiles criteria */
    bool matchGetCriteria(const ChargingProfileInfo& info,
                          ocpp::types::Optional<int> evse_id,
                          const ocpp::types::ocpp20::ChargingProfileCriterionType& criteria) const;

    /** @brief Check if a profile is currently valid */
    bool isProfileValid(const ocpp::types::ocpp20::ChargingProfileType& profile) const;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_PROFILEDATABASE20_H
