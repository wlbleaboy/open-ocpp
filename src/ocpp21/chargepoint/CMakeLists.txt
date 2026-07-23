######################################################
#            OCPP 2.1 chargepoint library           #
######################################################

# Library target
add_library(chargepoint21 OBJECT
    ChargePoint21.cpp

    authent/AuthentManager21.cpp
    authent/AuthentCache21.cpp
    authent/AuthentLocalList21.cpp
    authent/AuthentUtils21.cpp
    datatransfer/DataTransferManager21.cpp
    devicemodel/DeviceModelManager21.cpp
    devicemodel/DeviceModelMessagesManager21.cpp
    display/DisplayManager21.cpp
    maintenance/MaintenanceManager21.cpp
    metervalues/MeterValuesManager21.cpp
    monitoring/MonitoringManager21.cpp
    notify/NotifyManager21.cpp
    requestfifo/RequestFifo21.cpp
    requestfifo/RequestFifoManager21.cpp
    reservation/ReservationManager21.cpp
    security/SecurityManager21.cpp
    status/StatusManager21.cpp
    smartcharging/SmartChargingManager21.cpp
    tariff/TariffManager21.cpp
    transaction/TransactionManager21.cpp
    trigger/TriggerMessageManager21.cpp
)

# Exported includes
target_include_directories(chargepoint21 PUBLIC interface . authent datatransfer devicemodel display maintenance metervalues monitoring notify requestfifo reservation security status smartcharging tariff transaction trigger)

# Dependencies
target_link_libraries(chargepoint21 PUBLIC
    config
    database
    messages21
    rpc
    helpers
    log
    version
    x509
)
