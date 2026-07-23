/*
MIT License

Copyright (c) 2020 Cedric Jimenez
*/

#include "ChargerManager21.h"

#include "ChargePointDemoConfig.h"
#include "IChargePoint20.h"
#include "ReasonEnumType20.h"
#include "TriggerReasonEnumType20.h"
#include "WorkerThreadPool.h"

#include <chrono>
#include <iostream>
#include <thread>

using namespace ocpp::chargepoint::ocpp20;
using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

std::unique_ptr<IChargerManager> IChargerManager::create()
{
    return std::make_unique<ChargerManager21>();
}

ChargerManager21::ChargerManager21()
    : m_charge_point(nullptr),
      m_config(nullptr),
      m_setpoint_manager(),
      m_connected(false),
      m_registered(false),
      m_mutex(),
      m_transactions()
{
}

ChargerManager21::~ChargerManager21() = default;

namespace
{
unsigned int getUIntConfig(ChargePointDemoConfig& config, const std::string& section, const std::string& key, unsigned int default_value)
{
    ocpp::helpers::IniFile::Value value = config.getConfig(section, key);
    return value.isUInt() ? value.toUInt() : default_value;
}
} // namespace

void ChargerManager21::setChargePoint(IChargePoint20& charge_point, ChargePointDemoConfig& config)
{
    const unsigned int evse_count =
        getUIntConfig(config, "LocalTest", "EvseCount", 2u);
    const unsigned int max_charge_point_current =
        getUIntConfig(config, "LocalTest", "MaxChargingStationCurrent", 32u);
    const unsigned int max_evse_current =
        getUIntConfig(config, "LocalTest", "MaxEvseCurrent", 32u);

    m_config       = &config;
    m_charge_point = &charge_point;
    m_setpoint_manager =
        std::make_unique<SetpointManager>(charge_point, evse_count, max_charge_point_current, max_evse_current);

    std::cout << "[ChargerManager21] setpoints config: evseCount=" << evse_count
              << " stationMaxCurrent=" << max_charge_point_current
              << " evseMaxCurrent=" << max_evse_current << std::endl;
}

void ChargerManager21::connectionStateChanged(bool isConnected)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connected = isConnected;
    if (!isConnected)
    {
        m_registered = false;
    }
    std::cout << "[ChargerManager21] connectionStateChanged: " << isConnected << std::endl;
}

void ChargerManager21::connectionFailed()
{
    std::cout << "[ChargerManager21] connectionFailed" << std::endl;
}

void ChargerManager21::bootNotification(RegistrationStatusEnumType status, const DateTime& datetime)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_registered = (status == RegistrationStatusEnumType::Accepted);
    std::cout << "[ChargerManager21] bootNotification: " << RegistrationStatusEnumTypeHelper.toString(status) << " - "
              << datetime.str() << std::endl;
}

bool ChargerManager21::remoteStartTransactionRequested(unsigned int evse_id, int remote_start_id, const IdTokenType& id_token)
{
    std::cout << "[ChargerManager21] remoteStartTransactionRequested: evse=" << evse_id
              << " remoteStartId=" << remote_start_id << " token=" << id_token.idToken.str() << std::endl;

    IChargePoint20* charge_point = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        charge_point = m_charge_point;
    }
    if (!charge_point)
    {
        return false;
    }

    charge_point->getWorkerPool().run<void>(
        [this, evse_id, remote_start_id, id_token] { startRemoteTransaction(evse_id, remote_start_id, id_token); });
    return true;
}

bool ChargerManager21::remoteStopTransactionRequested(const std::string& transaction_id)
{
    std::cout << "[ChargerManager21] remoteStopTransactionRequested: transactionId=" << transaction_id << std::endl;

    IChargePoint20* charge_point = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        charge_point = m_charge_point;
    }
    if (!charge_point)
    {
        return false;
    }

    charge_point->getWorkerPool().run<void>([this, transaction_id] { stopRemoteTransaction(transaction_id); });
    return true;
}

void ChargerManager21::installFirmware(const std::string& firmware_file)
{
    std::cout << "[ChargerManager21] installFirmware: " << firmware_file << std::endl;
}

bool ChargerManager21::resetRequested(ResetEnumType reset_type)
{
    std::cout << "[ChargerManager21] resetRequested: " << ResetEnumTypeHelper.toString(reset_type) << std::endl;
    return true;
}

SetVariableStatusEnumType ChargerManager21::setConfiguration(const std::string& key, const std::string& value)
{
    std::cout << "[ChargerManager21] setConfiguration: " << key << "=" << value << std::endl;
    return SetVariableStatusEnumType::Accepted;
}

void ChargerManager21::sendLocalList(std::vector<std::string> localList)
{
    std::cout << "[ChargerManager21] sendLocalList: items=" << localList.size() << std::endl;
}

UnlockStatusEnumType ChargerManager21::unlockConnectorRequested(unsigned int evse_id, unsigned int connector_id)
{
    std::cout << "[ChargerManager21] unlockConnectorRequested: evse=" << evse_id << " connector=" << connector_id << std::endl;
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& transaction : m_transactions)
    {
        if ((transaction.second.evse_id == evse_id) && (transaction.second.connector_id == connector_id))
        {
            return UnlockStatusEnumType::OngoingAuthorizedTransaction;
        }
    }
    return UnlockStatusEnumType::Unlocked;
}

DataTransferStatusEnumType ChargerManager21::dataTransferRequested(const std::string& vendor_id,
                                                                   const std::string& message_id,
                                                                   const std::string& request_data,
                                                                   std::string&       response_data)
{
    std::cout << "[ChargerManager21] dataTransferRequested: vendor=" << vendor_id << " message=" << message_id
              << " data=" << request_data << std::endl;
    response_data.clear();
    return DataTransferStatusEnumType::Accepted;
}

ChangeAvailabilityStatusEnumType ChargerManager21::changeAvailability(unsigned int evse_id, OperationalStatusEnumType availability)
{
    std::cout << "[ChargerManager21] changeAvailability: evse=" << evse_id
              << " availability=" << OperationalStatusEnumTypeHelper.toString(availability) << std::endl;
    return ChangeAvailabilityStatusEnumType::Accepted;
}

void ChargerManager21::changeReserveStatus(unsigned int evse_id, unsigned int isReserved)
{
    std::cout << "[ChargerManager21] changeReserveStatus: evse=" << evse_id << " reserved=" << isReserved << std::endl;
}

void ChargerManager21::isChargerAvailableSchedule()
{
    std::cout << "[ChargerManager21] isChargerAvailableSchedule" << std::endl;
}

ocpp::helpers::IniFile::Value ChargerManager21::getConfigureValue(std::string section, const std::string& key)
{
    if (m_config)
    {
        return m_config->getConfig(section, key);
    }
    return ocpp::helpers::IniFile::Value();
}

void ChargerManager21::startRemoteTransaction(unsigned int evse_id, int remote_start_id, IdTokenType id_token)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    IChargePoint20* charge_point = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        charge_point = m_charge_point;
    }
    if (!charge_point)
    {
        return;
    }

    const unsigned int connector_id = 1u;
    std::string transaction_id;
    if (charge_point->startTransaction(evse_id,
                                       connector_id,
                                       id_token,
                                       TriggerReasonEnumType::RemoteStart,
                                       remote_start_id,
                                       transaction_id))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_transactions[transaction_id] = {evse_id, connector_id, id_token.idToken.str()};
        if (m_setpoint_manager)
        {
            m_setpoint_manager->setEvseCharging(evse_id, true);
        }
        std::cout << "[ChargerManager21] remote transaction started: " << transaction_id << std::endl;
    }
    else
    {
        std::cout << "[ChargerManager21] unable to start remote transaction" << std::endl;
    }
}

void ChargerManager21::stopRemoteTransaction(std::string transaction_id)
{
    IChargePoint20* charge_point = nullptr;
    std::string id_token;
    unsigned int evse_id = 0u;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        charge_point = m_charge_point;
        auto transaction = m_transactions.find(transaction_id);
        if (transaction != m_transactions.end())
        {
            id_token = transaction->second.id_token;
            evse_id  = transaction->second.evse_id;
        }
    }
    if (!charge_point)
    {
        return;
    }

    if (charge_point->stopTransaction(transaction_id, id_token, ReasonEnumType::Remote))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_transactions.erase(transaction_id);
        if (m_setpoint_manager)
        {
            m_setpoint_manager->setEvseCharging(evse_id, false);
        }
        std::cout << "[ChargerManager21] remote transaction stopped: " << transaction_id << std::endl;
    }
    else
    {
        std::cout << "[ChargerManager21] unable to stop remote transaction: " << transaction_id << std::endl;
    }
}
