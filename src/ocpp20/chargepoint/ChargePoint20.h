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

#ifndef OPENOCPP_OCPP20_CHARGEPOINT20_H
#define OPENOCPP_OCPP20_CHARGEPOINT20_H

#include "GenericMessageHandler.h"
#include "GenericMessageSender.h"
#include "IChargePoint20.h"
#include "InternalConfigManager.h"
#include "Logger.h"
#include "MessagesConverter20.h"
#include "MessagesValidator20.h"
#include "RpcClient.h"
#include "Connectors20.h"
#include "IDeviceModel20.h"
#include "Timer.h"

#include <atomic>
#include <memory>

namespace ocpp
{
// Forward declarations
namespace messages
{
class MessageDispatcher;
class GenericMessageSender;
} // namespace messages
namespace websockets
{
class IWebsocketClient;
}
namespace chargepoint
{
namespace ocpp20
{

class AuthentManager20;
class DataTransferManager20;
class DeviceModelMessagesManager20;
class MaintenanceManager20;
class MeterValuesManager20;
class DeviceModelManager;
class MonitoringManager20;
class NotifyManager20;
class RequestFifo20;
class RequestFifoManager20;
class ReservationManager20;
class SecurityManager20;
class SmartChargingManager20;
class TransactionManager20;
class StatusManager;
class TriggerMessageManager;

/** @brief Charge point implementation */
class ChargePoint20
    : public IChargePoint20,
      public ocpp::rpc::IRpc::IListener,
      public ocpp::rpc::IRpc::ISpy,
      public ocpp::rpc::RpcClient::IListener,

      // Central System messages
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ClearDisplayMessageReq,
                                                   ocpp::messages::ocpp20::ClearDisplayMessageConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::CustomerInformationReq,
                                                   ocpp::messages::ocpp20::CustomerInformationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::GetDisplayMessagesReq,
                                                   ocpp::messages::ocpp20::GetDisplayMessagesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::SetDisplayMessageReq,
                                                   ocpp::messages::ocpp20::SetDisplayMessageConf>
{
  public:
    /** @brief Constructor */
    ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                  IChargePointEventsHandler20&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter);

    /** @brief Constructor with internally owned device model */
    ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                  std::unique_ptr<DeviceModelManager>&&                          device_model_manager,
                  IChargePointEventsHandler20&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter);

    /** @brief Constructor with device model */
    ChargePoint20(const ocpp::config::IChargePointConfig20&                      stack_config,
                  IDeviceModel&                                                  device_model,
                  IChargePointEventsHandler20&                                   events_handler,
                  std::shared_ptr<ocpp::helpers::ITimerPool>                     timer_pool,
                  std::shared_ptr<ocpp::helpers::WorkerThreadPool>               worker_pool,
                  std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20>&& messages_converter);

    /** @brief Destructor */
    virtual ~ChargePoint20();

    // IChargePoint20 interface

    /** @copydoc ocpp::helpers::ITimerPool& IChargePoint20::getTimerPool() */
    ocpp::helpers::ITimerPool& getTimerPool() override { return *m_timer_pool.get(); }

    /** @copydoc ocpp::helpers::WorkerThreadPool& IChargePoint20::getWorkerPool() */
    ocpp::helpers::WorkerThreadPool& getWorkerPool() override { return *m_worker_pool.get(); }

    /** @copydoc INotifyManager20& IChargePoint20::getNotifyManager() */
    INotifyManager20& getNotifyManager() override;

    /** @copydoc ISecurityManager20& IChargePoint20::getSecurityManager() */
    ISecurityManager20& getSecurityManager() override;

    /** @copydoc ISmartChargingManager20& IChargePoint20::getSmartChargingManager() */
    ISmartChargingManager20& getSmartChargingManager() override;

    /** @copydoc ITransactionManager20& IChargePoint20::getTransactionManager() */
    ITransactionManager20& getTransactionManager() override;

    /** @copydoc bool IChargePoint20::resetData() */
    bool resetData() override;

    /** @copydoc bool IChargePoint20::start() */
    bool start() override;

    /** @copydoc bool IChargePoint20::stop() */
    bool stop() override;

    /** @copydoc bool IChargePoint20::reconnect() */
    bool reconnect() override;

    /** @copydoc bool IChargePoint20::dataTransfer(const std::string&, const std::string&, const std::string&, DataTransferStatusEnumType&, std::string&, std::string&, std::string&) */
    bool dataTransfer(const std::string&                               vendor_id,
                      const std::string&                               message_id,
                      const std::string&                               request_data,
                      ocpp::types::ocpp20::DataTransferStatusEnumType& status,
                      std::string&                                     response_data,
                      std::string&                                     error,
                      std::string&                                     message) override;

    /** @copydoc bool IChargePoint20::authorize(const ocpp::types::ocpp20::IdTokenType&, ocpp::types::ocpp20::IdTokenInfoType&, std::string&, std::string&) */
    bool authorize(const ocpp::types::ocpp20::IdTokenType& id_token,
                   ocpp::types::ocpp20::IdTokenInfoType&   token_info,
                   std::string&                            error,
                   std::string&                            message) override;

    /** @copydoc bool IChargePoint20::startTransaction(unsigned int,
     *                                                 unsigned int,
     *                                                 const ocpp::types::ocpp20::IdTokenType&,
     *                                                 ocpp::types::ocpp20::TriggerReasonEnumType,
     *                                                 std::string&)
     */
    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp20::IdTokenType&     id_token,
                          ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                          std::string&                                transaction_id) override;

    /** @copydoc bool IChargePoint20::startTransaction(unsigned int,
     *                                                 unsigned int,
     *                                                 const ocpp::types::ocpp20::IdTokenType&,
     *                                                 ocpp::types::ocpp20::TriggerReasonEnumType,
     *                                                 int,
     *                                                 std::string&)
     */
    bool startTransaction(unsigned int                                evse_id,
                          unsigned int                                connector_id,
                          const ocpp::types::ocpp20::IdTokenType&     id_token,
                          ocpp::types::ocpp20::TriggerReasonEnumType trigger_reason,
                          int                                         remote_start_id,
                          std::string&                                transaction_id) override;

    /** @copydoc bool IChargePoint20::startTransaction(unsigned int, unsigned int, const std::string&, std::string&) */
    bool startTransaction(unsigned int       evse_id,
                          unsigned int       connector_id,
                          const std::string& id_token,
                          std::string&       transaction_id) override;

    /** @copydoc bool IChargePoint20::updateTransaction(const std::string&,
     *                                                  ocpp::types::ocpp20::TriggerReasonEnumType,
     *                                                  const std::vector<ocpp::types::ocpp20::MeterValueType>&)
     */
    bool updateTransaction(const std::string&                                      transaction_id,
                           ocpp::types::ocpp20::TriggerReasonEnumType              trigger_reason,
                           const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    /** @copydoc bool IChargePoint20::hasActiveTransaction(const std::string&) */
    bool hasActiveTransaction(const std::string& transaction_id) override;

    /** @copydoc bool IChargePoint20::stopTransaction(const std::string&,
     *                                                ocpp::types::ocpp20::ReasonEnumType,
     *                                                ocpp::types::ocpp20::TriggerReasonEnumType,
     *                                                const ocpp::types::ocpp20::IdTokenType*,
     *                                                const std::vector<ocpp::types::ocpp20::MeterValueType>&)
     */
    bool stopTransaction(const std::string&                                      transaction_id,
                         ocpp::types::ocpp20::ReasonEnumType                    reason,
                         ocpp::types::ocpp20::TriggerReasonEnumType             trigger_reason,
                         const ocpp::types::ocpp20::IdTokenType*                id_token,
                         const std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    /** @copydoc bool IChargePoint20::stopTransaction(const std::string&, const std::string&, ocpp::types::ocpp20::ReasonEnumType) */
    bool stopTransaction(const std::string&                   transaction_id,
                         const std::string&                   id_token,
                         ocpp::types::ocpp20::ReasonEnumType reason) override;

    /** @copydoc void IChargePoint20::startPeriodicMeterValues(unsigned int, std::chrono::seconds) */
    void startPeriodicMeterValues(unsigned int evse_id, std::chrono::seconds interval) override;

    /** @copydoc void IChargePoint20::stopPeriodicMeterValues(unsigned int) */
    void stopPeriodicMeterValues(unsigned int evse_id) override;

    /** @copydoc void IChargePoint20::startTransactionSampledMeterValues(const std::string&, unsigned int, std::chrono::seconds) */
    void startTransactionSampledMeterValues(const std::string& transaction_id,
                                            unsigned int       evse_id,
                                            std::chrono::seconds interval) override;

    /** @copydoc void IChargePoint20::stopTransactionSampledMeterValues(const std::string&) */
    void stopTransactionSampledMeterValues(const std::string& transaction_id) override;

    /** @copydoc void IChargePoint20::getTxStopMeterValues(const std::string&,
     *                                                     std::vector<ocpp::types::ocpp20::MeterValueType>&)
     */
    void getTxStopMeterValues(const std::string& transaction_id,
                              std::vector<ocpp::types::ocpp20::MeterValueType>& meter_values) override;

    /** @copydoc bool IChargePoint20::getSetpoint(unsigned int,
     *                                            ocpp::types::Optional<ISmartChargingManager20::SmartChargingSetpoint>&,
     *                                            ocpp::types::Optional<ISmartChargingManager20::SmartChargingSetpoint>&,
     *                                            ocpp::types::ocpp20::ChargingRateUnitEnumType)
     */
    bool getSetpoint(unsigned int                                                             evse_id,
                     ocpp::types::Optional<ISmartChargingManager20::SmartChargingSetpoint>&    charging_station_setpoint,
                     ocpp::types::Optional<ISmartChargingManager20::SmartChargingSetpoint>&    evse_setpoint,
                     ocpp::types::ocpp20::ChargingRateUnitEnumType unit = ocpp::types::ocpp20::ChargingRateUnitEnumType::A) override;

    // RpcClient::IListener interface

    /** @copydoc void RpcClient::IListener::rpcClientConnected() */
    void rpcClientConnected() override;

    /** @copydoc void RpcClient::IListener::rpcClientFailed() */
    void rpcClientFailed() override;

    // IRpc::IListener interface

    /** @copydoc void IRpc::IListener::rpcDisconnected() */
    void rpcDisconnected() override;

    /** @copydoc void IRpc::IListener::rpcError() */
    void rpcError() override;

    /** @copydoc void IRpc::IListener::rpcCallReceived(const std::string&,
                                                       const rapidjson::Value&,
                                                       rapidjson::Document&,
                                                       std::string&,
                                                       std::string&) */
    bool rpcCallReceived(const std::string&      action,
                         const rapidjson::Value& payload,
                         rapidjson::Document&    response,
                         std::string&            error_code,
                         std::string&            error_message) override;

    // IRpc::ISpy interface

    /** @copydoc void IRpc::ISpy::rcpMessageReceived(const std::string&) */
    void rcpMessageReceived(const std::string& msg) override;

    /** @copydoc void IRpc::ISpy::rcpMessageSent(const std::string&) */
    void rcpMessageSent(const std::string& msg) override;

    /** @copydoc bool IBasicChargePoint20::statusNotification(unsigned int, unsigned int, ocpp::types::ocpp20::ConnectorStatusEnumType) */
    bool statusNotification(unsigned int evse_id, unsigned int connector_id, ocpp::types::ocpp20::ConnectorStatusEnumType status) override;

    // OCPP handlers

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::ClearDisplayMessageReq& request,
                       ocpp::messages::ocpp20::ClearDisplayMessageConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::CustomerInformationReq& request,
                       ocpp::messages::ocpp20::CustomerInformationConf&      response,
                       std::string&                                          error_code,
                       std::string&                                          error_message) override;

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::GetDisplayMessagesReq& request,
                       ocpp::messages::ocpp20::GetDisplayMessagesConf&      response,
                       std::string&                                         error_code,
                       std::string&                                         error_message) override;

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::SetDisplayMessageReq& request,
                       ocpp::messages::ocpp20::SetDisplayMessageConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

  private:
    /** @brief Stack configuration */
    const ocpp::config::IChargePointConfig20& m_stack_config;
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Owned device model when the stack creates it internally */
    std::unique_ptr<DeviceModelManager> m_device_model_manager;
    /** @brief Device model */
    IDeviceModel* m_device_model;

    /** @brief Timer pool */
    std::shared_ptr<ocpp::helpers::ITimerPool> m_timer_pool;
    /** @brief Worker thread pool */
    std::shared_ptr<ocpp::helpers::WorkerThreadPool> m_worker_pool;

    /** @brief Database */
    ocpp::database::Database m_database;
    /** @brief Internal configuration manager */
    ocpp::config::InternalConfigManager m_internal_config;

    /** @brief Messages converter */
    std::unique_ptr<ocpp::messages::ocpp20::MessagesConverter20> m_messages_converter;
    /** @brief Messaes validator */
    ocpp::messages::ocpp20::MessagesValidator20 m_messages_validator;
    /** @brief Indicate that a stop process is in progress */
    std::atomic<bool> m_stop_in_progress;
    /** @brief Indicate that a reconnection process has been scheduled */
    std::atomic<bool> m_reconnect_scheduled;
    /** @brief Current RPC connection state */
    std::atomic<bool> m_rpc_connected;

    /** @brief Websocket s*/
    std::unique_ptr<ocpp::websockets::IWebsocketClient> m_ws_client;
    /** @brief RPC client */
    std::unique_ptr<ocpp::rpc::RpcClient> m_rpc_client;
    /** @brief Message dispatcher */
    std::unique_ptr<ocpp::messages::MessageDispatcher> m_msg_dispatcher;
    /** @brief Message sender */
    std::unique_ptr<ocpp::messages::GenericMessageSender> m_msg_sender;
    /** @brief Authentication manager */
    std::unique_ptr<AuthentManager20> m_authent_manager;
    /** @brief Charge point connectors */
    Connectors m_connectors;
    /** @brief Trigger message manager */
    std::unique_ptr<TriggerMessageManager> m_trigger_manager;
    /** @brief Status manager */
    std::unique_ptr<StatusManager> m_status_manager;
    /** @brief Offline request FIFO */
    std::unique_ptr<RequestFifo20> m_requests_fifo;
    /** @brief Offline request FIFO manager */
    std::unique_ptr<RequestFifoManager20> m_requests_fifo_manager;
    /** @brief Device model messages manager */
    std::unique_ptr<DeviceModelMessagesManager20> m_device_model_messages_manager;
    /** @brief Monitoring manager */
    std::unique_ptr<MonitoringManager20> m_monitoring_manager;
    /** @brief Maintenance manager */
    std::unique_ptr<MaintenanceManager20> m_maintenance_manager;
    /** @brief Reservation manager */
    std::unique_ptr<ReservationManager20> m_reservation_manager;
    /** @brief Smart charging manager */
    std::unique_ptr<SmartChargingManager20> m_smart_charging_manager;
    /** @brief Transaction manager */
    std::unique_ptr<TransactionManager20> m_transaction_manager;
    /** @brief Meter values manager */
    std::unique_ptr<MeterValuesManager20> m_meter_values_manager;
    /** @brief Notify messages manager */
    std::unique_ptr<NotifyManager20> m_notify_manager;
    /** @brief Security manager */
    std::unique_ptr<SecurityManager20> m_security_manager;
    /** @brief Data transfer manager */
    std::unique_ptr<DataTransferManager20> m_data_transfer_manager;

    /** @brief Uptime timer */
    ocpp::helpers::Timer m_uptime_timer;
    /** @brief Uptime in seconds */
    unsigned int m_uptime;
    /** @brief Disconnected time in seconds */
    unsigned int m_disconnected_time;
    /** @brief Total uptime in seconds */
    unsigned int m_total_uptime;
    /** @brief Total disconnected time in seconds */
    unsigned int m_total_disconnected_time;

    /** @brief Initialize the database */
    void initDatabase();
    /** @brief Process uptime */
    void processUptime();
    /** @brief Save the uptime counter in database */
    void saveUptime();

    /** @brief Schedule a reconnection to the Central System */
    void scheduleReconnect();
    /** @brief Start the connection process to the Central System */
    bool doConnect();

    /**
     * @brief Execute a call request
     * @param action RPC action for the request
     * @param request Request payload
     * @param response Response payload
     * @param error Error (Empty if not a CallError)
     * @param message Error message (Empty if not a CallError)
     * @return true if the request has been sent and a response has been received, false otherwise
     */
    template <typename RequestType, typename ResponseType>
    bool call(const std::string& action, const RequestType& request, ResponseType& response, std::string& error, std::string& message)
    {
        bool ret = false;

        std::string identifier = m_stack_config.chargePointIdentifier();
        LOG_DEBUG << "[" << identifier << "] - " << action;

        if (m_msg_sender && !m_stop_in_progress)
        {
            ocpp::messages::CallResult res = m_msg_sender->call(action, request, response, error, message);
            if (res != ocpp::messages::CallResult::Ok)
            {
                LOG_ERROR << "[" << identifier << "] - " << action << " => "
                          << (res == ocpp::messages::CallResult::Failed ? "Timeout" : "Error");
            }
            else
            {
                ret = true;
            }
        }

        return ret;
    }
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_CHARGEPOINT20_H
