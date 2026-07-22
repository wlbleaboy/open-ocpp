#ifndef ICHARGERMANAGER_H
#define ICHARGERMANAGER_H

#include "ChangeAvailabilityStatusEnumType20.h"
#include "DataTransferStatusEnumType20.h"
#include "DateTime.h"
#include "IdTokenType20.h"
#include "IniFile.h"
#include "OperationalStatusEnumType20.h"
#include "RegistrationStatusEnumType20.h"
#include "ResetEnumType20.h"
#include "SetVariableStatusEnumType20.h"
#include "UnlockStatusEnumType20.h"

#include <memory>
#include <string>
#include <vector>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{
class IChargePoint20;
}
} // namespace chargepoint
} // namespace ocpp

class ChargePointDemoConfig;

class IChargerManager{
public:
    virtual ~IChargerManager(){}
    
    static std::unique_ptr<IChargerManager> create();

    virtual void setChargePoint(ocpp::chargepoint::ocpp20::IChargePoint20& charge_point, ChargePointDemoConfig& config) = 0;
    virtual void connectionStateChanged(bool isConnected) = 0;
    virtual void connectionFailed() = 0;
    virtual void bootNotification(ocpp::types::ocpp20::RegistrationStatusEnumType status, const ocpp::types::DateTime& datetime) = 0;
    virtual bool remoteStartTransactionRequested(unsigned int evse_id,
                                                 int remote_start_id,
                                                 const ocpp::types::ocpp20::IdTokenType& id_token) = 0;
    virtual bool remoteStopTransactionRequested(const std::string& transaction_id) = 0;
    virtual void installFirmware(const std::string& firmware_file) = 0;
    virtual bool resetRequested(ocpp::types::ocpp20::ResetEnumType reset_type) = 0;
    virtual ocpp::types::ocpp20::SetVariableStatusEnumType setConfiguration(const std::string& key, const std::string& value) = 0;
    virtual void sendLocalList(std::vector<std::string> localList) = 0;
    virtual ocpp::types::ocpp20::UnlockStatusEnumType unlockConnectorRequested(unsigned int evse_id, unsigned int connector_id) = 0;
    virtual ocpp::types::ocpp20::DataTransferStatusEnumType dataTransferRequested(const std::string& vendor_id,
                                                                                  const std::string& message_id,
                                                                                  const std::string& request_data,
                                                                                  std::string&       response_data) = 0;

    virtual ocpp::types::ocpp20::ChangeAvailabilityStatusEnumType changeAvailability(
        unsigned int evse_id,
        ocpp::types::ocpp20::OperationalStatusEnumType availability) = 0;

    virtual void changeReserveStatus(unsigned int evse_id, unsigned int isReserved) = 0;
    virtual void isChargerAvailableSchedule() = 0;

    virtual ocpp::helpers::IniFile::Value getConfigureValue( std::string section, const std::string& key ) = 0;
};

#endif
