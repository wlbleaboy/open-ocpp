######################################################
#            OCPP chargepoint library                #
######################################################


# Library target
add_library(chargepoint20 OBJECT
    ChargePoint20.cpp
    BasicChargePoint20.cpp

    authent/AuthentCache20.cpp
    authent/AuthentLocalList20.cpp
    authent/AuthentManager20.cpp
    authent/AuthentUtils20.cpp
    connector/Connectors20.cpp
    datatransfer/DataTransferManager20.cpp
    devicemodel/DeviceModelManager20.cpp
    devicemodel/DeviceModelMessagesManager20.cpp
    maintenance/MaintenanceManager20.cpp
    metervalues/MeterValuesManager20.cpp
    monitoring/MonitoringManager20.cpp
    notify/NotifyManager20.cpp
    requestfifo/RequestFifo20.cpp
    requestfifo/RequestFifoManager20.cpp
    reservation/ReservationManager20.cpp
    security/SecurityManager20.cpp
    smartcharging/ProfileDatabase20.cpp
    smartcharging/SmartChargingManager20.cpp
    status/StatusManager20.cpp
    transaction/TransactionManager20.cpp
    trigger/TriggerMessageManager20.cpp
)

# Exported includes
target_include_directories(chargepoint20 PUBLIC interface devicemodel connector datatransfer authent transaction requestfifo metervalues reservation maintenance notify security smartcharging)

# Private includes
target_include_directories(chargepoint20 PRIVATE config monitoring status trigger)

# Dependencies
target_link_libraries(chargepoint20 PUBLIC
    config
    database
    messages20
    rpc
    helpers
    log
    version
    x509
)
