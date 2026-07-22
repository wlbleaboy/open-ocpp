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

#ifndef OPENOCPP_OCPP20_SMARTCHARGINGMANAGER20_H
#define OPENOCPP_OCPP20_SMARTCHARGINGMANAGER20_H

#include "ClearChargingProfile20.h"
#include "GenericMessageHandler.h"
#include "GetChargingProfiles20.h"
#include "GetCompositeSchedule20.h"
#include "ISmartChargingManager20.h"
#include "ProfileDatabase20.h"
#include "SetChargingProfile20.h"

#include <mutex>

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages

namespace database
{
class Database;
} // namespace database

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 smart charging requests for the charge point */
class SmartChargingManager20 : public ISmartChargingManager20,
                               public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ClearChargingProfileReq,
                                                   ocpp::messages::ocpp20::ClearChargingProfileConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetChargingProfilesReq,
                                                   ocpp::messages::ocpp20::GetChargingProfilesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetCompositeScheduleReq,
                                                   ocpp::messages::ocpp20::GetCompositeScheduleConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetChargingProfileReq,
                                                   ocpp::messages::ocpp20::SetChargingProfileConf>
{
  public:
    /** @brief Constructor */
    SmartChargingManager20(ocpp::database::Database&                       database,
                           IChargePointEventsHandler20&                    events_handler,
                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                           ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                           ocpp::messages::GenericMessageSender&           msg_sender);

    /** @brief Destructor */
    virtual ~SmartChargingManager20();

    // ISmartChargingManager20 interface

    bool clearedChargingLimit(const ocpp::messages::ocpp20::ClearedChargingLimitReq& request,
                              ocpp::messages::ocpp20::ClearedChargingLimitConf&      response,
                              std::string&                                           error,
                              std::string&                                           message) override;

    bool getSetpoint(unsigned int                                  evse_id,
                     ocpp::types::Optional<SmartChargingSetpoint>& charging_station_setpoint,
                     ocpp::types::Optional<SmartChargingSetpoint>& evse_setpoint,
                     ocpp::types::ocpp20::ChargingRateUnitEnumType unit = ocpp::types::ocpp20::ChargingRateUnitEnumType::A) override;

    bool installTxProfile(unsigned int evse_id, const ocpp::types::ocpp20::ChargingProfileType& profile) override;

    void assignPendingTxProfiles(unsigned int evse_id, const std::string& transaction_id) override;

    void clearTxProfiles(unsigned int evse_id) override;

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::ClearChargingProfileReq& request,
                       ocpp::messages::ocpp20::ClearChargingProfileConf&      response,
                       std::string&                                           error_code,
                       std::string&                                           error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetChargingProfilesReq& request,
                       ocpp::messages::ocpp20::GetChargingProfilesConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::GetCompositeScheduleReq& request,
                       ocpp::messages::ocpp20::GetCompositeScheduleConf&      response,
                       std::string&                                           error_code,
                       std::string&                                           error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::SetChargingProfileReq& request,
                       ocpp::messages::ocpp20::SetChargingProfileConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;

    /** @brief OCPP message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;

    /** @brief Charging profile database */
    ProfileDatabase20 m_profile_db;

    /** @brief Synchronize profile access */
    std::mutex m_mutex;

    /** @brief Send a ReportChargingProfiles request */
    bool reportChargingProfiles(int request_id, const std::vector<ProfileDatabase20::ChargingProfileInfo>& profiles);

    /** @brief Build a composite schedule from active profiles */
    bool buildCompositeSchedule(int evse_id,
                                int duration,
                                ocpp::types::Optional<ocpp::types::ocpp20::ChargingRateUnitEnumType> requested_unit,
                                ocpp::types::ocpp20::CompositeScheduleType& schedule);

    /** @brief Extract the current setpoint from active profiles */
    bool getActiveSetpoint(unsigned int evse_id,
                           bool station_profile,
                           ocpp::types::ocpp20::ChargingRateUnitEnumType unit,
                           SmartChargingSetpoint& setpoint);

    /** @brief Check if a charging profile contains a usable schedule */
    bool isProfileValid(const ocpp::types::ocpp20::ChargingProfileType& profile) const;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_SMARTCHARGINGMANAGER20_H
