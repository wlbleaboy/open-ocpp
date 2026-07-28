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

#ifndef OPENOCPP_OCPP21_CHARGEPOINT21_H
#define OPENOCPP_OCPP21_CHARGEPOINT21_H

#include "Database.h"
#include "GenericMessageHandler.h"
#include "GenericMessageSender.h"
#include "IChargePoint21.h"
#include "MessagesConverter21.h"
#include "MessagesValidator21.h"
#include "RpcClient.h"

#include <atomic>
#include <memory>
#include <string>

namespace ocpp
{
namespace websockets
{
class IWebsocketClient;
} // namespace websockets
namespace messages
{
class MessageDispatcher;
} // namespace messages
namespace helpers
{
class ITimerPool;
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp21
{

class AuthentManager21;
class DataTransferManager21;
class StatusManager21;
class TransactionManager21;
class MeterValuesManager21;
class SmartChargingManager21;
class TriggerMessageManager21;
class MaintenanceManager21;
class DeviceModelManager21;
class DeviceModelMessagesManager21;
class DisplayManager21;
class MonitoringManager21;
class NotifyManager21;
class RequestFifo21;
class RequestFifoManager21;
class ReservationManager21;
class SecurityManager21;
class TariffManager21;

/** @brief OCPP 2.1 charge point implementation */
class ChargePoint21 : public IChargePoint21,
public ocpp::rpc::IRpc::IListener,
public ocpp::rpc::IRpc::ISpy,
public ocpp::rpc::RpcClient::IListener
{
  public:
    /** @brief Constructor */
    ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                  IChargePointEventsHandler21&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter);

    /** @brief Constructor with internally owned device model */
    ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                  std::unique_ptr<DeviceModelManager21>&&                         device_model_manager,
                  IChargePointEventsHandler21&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter);

    /** @brief Constructor with device model */
    ChargePoint21(const ocpp::config::IChargePointConfig21&                      stack_config,
                  IDeviceModel21&                                                device_model,
                  IChargePointEventsHandler21&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21>&& messages_converter);

    /** @brief Destructor */
    ~ChargePoint21();

    ocpp::helpers::ITimerPool& getTimerPool() override;
    ocpp::helpers::WorkerThreadPool& getWorkerPool() override;
    MeterValuesManager21& getMeterValuesManager() override;
    INotifyManager21& getNotifyManager() override;
    ISecurityManager21& getSecurityManager() override;
    SmartChargingManager21& getSmartChargingManager() override;
    StatusManager21& getStatusManager() override;
    TransactionManager21& getTransactionManager() override;
    bool resetData() override;
    bool start() override;
    bool stop() override;
    bool reconnect() override;
    bool authorize(const ocpp::types::ocpp21::IdTokenType& id_token,
                   ocpp::types::ocpp21::IdTokenInfoType&   token_info,
                   std::string&                            error,
                   std::string&                            message) override;

    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp21::IdTokenType&     id_token,
                          ocpp::types::ocpp21::TriggerReasonEnumType trigger_reason,
                          std::string&                                transaction_id) override;
    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp21::IdTokenType&     id_token,
                          ocpp::types::ocpp21::TriggerReasonEnumType trigger_reason,
                          int                                         remote_start_id,
                          std::string&                                transaction_id) override;
    bool startTransaction(unsigned int       evse_id,
                          unsigned int       connector_id,
                          const std::string& id_token,
                          std::string&       transaction_id) override;
    bool updateTransaction(const std::string&                                      transaction_id,
                           ocpp::types::ocpp21::TriggerReasonEnumType              trigger_reason,
                           const std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values) override;
    bool hasActiveTransaction(const std::string& transaction_id) override;
    bool stopTransaction(const std::string&                                      transaction_id,
                         ocpp::types::ocpp21::ReasonEnumType                    reason,
                         ocpp::types::ocpp21::TriggerReasonEnumType             trigger_reason,
                         const ocpp::types::ocpp21::IdTokenType*                id_token,
                         const std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values) override;
    bool stopTransaction(const std::string&                   transaction_id,
                         const std::string&                   id_token,
                         ocpp::types::ocpp21::ReasonEnumType reason) override;
    void startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval) override;
    void stopPeriodicMeterValues(unsigned int evse_id) override;
    void startTransactionSampledMeterValues(const std::string& transaction_id,
                                            unsigned int       evse_id,
                                            std::chrono::seconds interval) override;
    void stopTransactionSampledMeterValues(const std::string& transaction_id) override;
    void getTxStopMeterValues(const std::string& transaction_id,
                              std::vector<ocpp::types::ocpp21::MeterValueType>& meter_values) override;
    bool getSetpoint(unsigned int                                               evse_id,
                     ocpp::types::Optional<SmartChargingManager21::SmartChargingSetpoint>& charging_station_setpoint,
                     ocpp::types::Optional<SmartChargingManager21::SmartChargingSetpoint>& evse_setpoint,
                     ocpp::types::ocpp21::ChargingRateUnitEnumType unit = ocpp::types::ocpp21::ChargingRateUnitEnumType::A) override;
    bool statusNotification(unsigned int                                 evse_id,
                            unsigned int                                 connector_id,
                            ocpp::types::ocpp21::ConnectorStatusEnumType status) override;

    bool call(const ocpp::messages::ocpp21::VatNumberValidationReq& request,
              ocpp::messages::ocpp21::VatNumberValidationConf&      response,
              std::string&                              error,
              std::string&                              message) override;

    void rpcClientConnected() override;
    void rpcClientFailed() override;
    void rpcDisconnected() override;
    void rpcError() override;
    bool rpcCallReceived(const std::string&      action,
                         const rapidjson::Value& payload,
                         rapidjson::Document&    response,
                         std::string&            error_code,
                         std::string&            error_message) override;
    void rcpMessageReceived(const std::string& msg) override;
    void rcpMessageSent(const std::string& msg) override;

  private:
    template <typename RequestType, typename ResponseType>
    bool sendCall(const std::string& action,
                  const RequestType& request,
                  ResponseType&      response,
                  std::string&       error,
                  std::string&       message)
    {
        bool ret = false;
        if (m_msg_sender)
        {
            ret = (m_msg_sender->call(action, request, response, error, message) == ocpp::messages::CallResult::Ok);
        }
        return ret;
    }

    void scheduleReconnect();
    bool doConnect();

    const ocpp::config::IChargePointConfig21& m_stack_config;
    IChargePointEventsHandler21&              m_events_handler;
    std::unique_ptr<DeviceModelManager21>     m_device_model_manager;
    IDeviceModel21*                           m_device_model;
    std::shared_ptr<ocpp::helpers::ITimerPool>       m_timer_pool;
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> m_worker_pool;
    ocpp::database::Database m_database;
    std::unique_ptr<ocpp::messages::ocpp21::MessagesConverter21> m_messages_converter;
    ocpp::messages::ocpp21::MessagesValidator21 m_messages_validator;
    std::atomic_bool m_stop_in_progress;
    std::atomic_bool m_reconnect_scheduled;
    std::atomic_bool m_rpc_connected;
    std::unique_ptr<ocpp::websockets::IWebsocketClient> m_ws_client;
    std::unique_ptr<ocpp::rpc::RpcClient> m_rpc_client;
    std::unique_ptr<ocpp::messages::MessageDispatcher> m_msg_dispatcher;
    std::unique_ptr<ocpp::messages::GenericMessageSender> m_msg_sender;
    std::unique_ptr<AuthentManager21> m_authent_manager;
    std::unique_ptr<DataTransferManager21> m_data_transfer_manager;
    std::unique_ptr<StatusManager21> m_status_manager;
    std::unique_ptr<TransactionManager21> m_transaction_manager;
    std::unique_ptr<MeterValuesManager21> m_meter_values_manager;
    std::unique_ptr<NotifyManager21> m_notify_manager;
    std::unique_ptr<SmartChargingManager21> m_smart_charging_manager;
    std::unique_ptr<TriggerMessageManager21> m_trigger_manager;
    std::unique_ptr<MaintenanceManager21> m_maintenance_manager;
    std::unique_ptr<DisplayManager21> m_display_manager;
    std::unique_ptr<MonitoringManager21> m_monitoring_manager;
    std::unique_ptr<RequestFifo21> m_request_fifo;
    std::unique_ptr<RequestFifoManager21> m_request_fifo_manager;
    std::unique_ptr<ReservationManager21> m_reservation_manager;
    std::unique_ptr<SecurityManager21> m_security_manager;
    std::unique_ptr<TariffManager21> m_tariff_manager;
    std::unique_ptr<DeviceModelMessagesManager21> m_device_model_messages_manager;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_CHARGEPOINT21_H
