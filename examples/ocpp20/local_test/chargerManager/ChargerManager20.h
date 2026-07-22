/*
MIT License

Copyright (c) 2020 Cedric Jimenez
*/

#ifndef CHARGERMANAGER20_H
#define CHARGERMANAGER20_H

#include "IChargerManager.h"
#include "SetpointManager.h"

#include <mutex>
#include <memory>
#include <unordered_map>

/** @brief Minimal OCPP 2.0.1 local-test charger manager */
class ChargerManager20 : public IChargerManager
{
  public:
    ChargerManager20();
    ~ChargerManager20() override;

    void setChargePoint(ocpp::chargepoint::ocpp20::IChargePoint20& charge_point, ChargePointDemoConfig& config) override;
    void connectionStateChanged(bool isConnected) override;
    void connectionFailed() override;
    void bootNotification(ocpp::types::ocpp20::RegistrationStatusEnumType status, const ocpp::types::DateTime& datetime) override;
    bool remoteStartTransactionRequested(unsigned int evse_id,
                                         int remote_start_id,
                                         const ocpp::types::ocpp20::IdTokenType& id_token) override;
    bool remoteStopTransactionRequested(const std::string& transaction_id) override;
    void installFirmware(const std::string& firmware_file) override;
    bool resetRequested(ocpp::types::ocpp20::ResetEnumType reset_type) override;
    ocpp::types::ocpp20::SetVariableStatusEnumType setConfiguration(const std::string& key, const std::string& value) override;
    void sendLocalList(std::vector<std::string> localList) override;
    ocpp::types::ocpp20::UnlockStatusEnumType unlockConnectorRequested(unsigned int evse_id, unsigned int connector_id) override;
    ocpp::types::ocpp20::DataTransferStatusEnumType dataTransferRequested(const std::string& vendor_id,
                                                                          const std::string& message_id,
                                                                          const std::string& request_data,
                                                                          std::string&       response_data) override;
    ocpp::types::ocpp20::ChangeAvailabilityStatusEnumType changeAvailability(
        unsigned int evse_id,
        ocpp::types::ocpp20::OperationalStatusEnumType availability) override;
    void changeReserveStatus(unsigned int evse_id, unsigned int isReserved) override;
    void isChargerAvailableSchedule() override;
    ocpp::helpers::IniFile::Value getConfigureValue(std::string section, const std::string& key) override;

  private:
    struct TransactionContext
    {
        unsigned int evse_id;
        unsigned int connector_id;
        std::string id_token;
    };

    ocpp::chargepoint::ocpp20::IChargePoint20* m_charge_point;
    ChargePointDemoConfig* m_config;
    std::unique_ptr<SetpointManager> m_setpoint_manager;
    bool m_connected;
    bool m_registered;
    std::mutex m_mutex;
    std::unordered_map<std::string, TransactionContext> m_transactions;

    void startRemoteTransaction(unsigned int evse_id, int remote_start_id, ocpp::types::ocpp20::IdTokenType id_token);
    void stopRemoteTransaction(std::string transaction_id);
};

#endif // CHARGERMANAGER20_H
