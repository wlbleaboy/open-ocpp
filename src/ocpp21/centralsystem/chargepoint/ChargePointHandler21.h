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

#ifndef OPENOCPP_OCPP21_CS_CHARGEPOINTHANDLER21_H
#define OPENOCPP_OCPP21_CS_CHARGEPOINTHANDLER21_H

#include "Logger.h"
#include "GenericMessageHandler.h"
#include "MessagesConverter21.h"
#include "Authorize21.h"
#include "BatterySwap21.h"
#include "BootNotification21.h"
#include "ClearedChargingLimit21.h"
#include "CostUpdated21.h"
#include "DataTransfer21.h"
#include "FirmwareStatusNotification21.h"
#include "Heartbeat21.h"
#include "LogStatusNotification21.h"
#include "MeterValues21.h"
#include "NotifyAllowedEnergyTransfer21.h"
#include "NotifyChargingLimit21.h"
#include "NotifyCustomerInformation21.h"
#include "NotifyDERAlarm21.h"
#include "NotifyDERStartStop21.h"
#include "NotifyDisplayMessages21.h"
#include "NotifyEVChargingNeeds21.h"
#include "NotifyEVChargingSchedule21.h"
#include "NotifyEvent21.h"
#include "NotifyMonitoringReport21.h"
#include "NotifyPeriodicEventStream21.h"
#include "NotifyPriorityCharging21.h"
#include "NotifyReport21.h"
#include "NotifySettlement21.h"
#include "NotifyWebPaymentStarted21.h"
#include "PublishFirmwareStatusNotification21.h"
#include "PullDynamicScheduleUpdate21.h"
#include "ReportChargingProfiles21.h"
#include "ReportDERControl21.h"
#include "ReservationStatusUpdate21.h"
#include "SecurityEventNotification21.h"
#include "SignCertificate21.h"
#include "StatusNotification21.h"
#include "TransactionEvent21.h"
#include "VatNumberValidation21.h"

namespace ocpp
{
namespace config
{
class ICentralSystemConfig21;
} // namespace config
namespace messages
{
class MessageDispatcher;

namespace ocpp21
{
class MessagesConverter21;
} // namespace ocpp21
} // namespace messages

namespace centralsystem
{
namespace ocpp21
{

class IChargePointRequestHandler21;

/** @brief Handler for charge point requests */
class ChargePointHandler21
    :
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::AuthorizeReq,
                                                   ocpp::messages::ocpp21::AuthorizeConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::BatterySwapReq,
                                                   ocpp::messages::ocpp21::BatterySwapConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::BootNotificationReq,
                                                   ocpp::messages::ocpp21::BootNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ClearedChargingLimitReq,
                                                   ocpp::messages::ocpp21::ClearedChargingLimitConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::CostUpdatedReq,
                                                   ocpp::messages::ocpp21::CostUpdatedConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::DataTransferReq,
                                                   ocpp::messages::ocpp21::DataTransferConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::FirmwareStatusNotificationReq,
                                                   ocpp::messages::ocpp21::FirmwareStatusNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::HeartbeatReq,
                                                   ocpp::messages::ocpp21::HeartbeatConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::LogStatusNotificationReq,
                                                   ocpp::messages::ocpp21::LogStatusNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::MeterValuesReq,
                                                   ocpp::messages::ocpp21::MeterValuesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyAllowedEnergyTransferReq,
                                                   ocpp::messages::ocpp21::NotifyAllowedEnergyTransferConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyChargingLimitReq,
                                                   ocpp::messages::ocpp21::NotifyChargingLimitConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyCustomerInformationReq,
                                                   ocpp::messages::ocpp21::NotifyCustomerInformationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyDERAlarmReq,
                                                   ocpp::messages::ocpp21::NotifyDERAlarmConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyDERStartStopReq,
                                                   ocpp::messages::ocpp21::NotifyDERStartStopConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyDisplayMessagesReq,
                                                   ocpp::messages::ocpp21::NotifyDisplayMessagesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyEVChargingNeedsReq,
                                                   ocpp::messages::ocpp21::NotifyEVChargingNeedsConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyEVChargingScheduleReq,
                                                   ocpp::messages::ocpp21::NotifyEVChargingScheduleConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyEventReq,
                                                   ocpp::messages::ocpp21::NotifyEventConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyMonitoringReportReq,
                                                   ocpp::messages::ocpp21::NotifyMonitoringReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyPeriodicEventStreamReq,
                                                   ocpp::messages::ocpp21::NotifyPeriodicEventStreamConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyPriorityChargingReq,
                                                   ocpp::messages::ocpp21::NotifyPriorityChargingConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyReportReq,
                                                   ocpp::messages::ocpp21::NotifyReportConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifySettlementReq,
                                                   ocpp::messages::ocpp21::NotifySettlementConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::NotifyWebPaymentStartedReq,
                                                   ocpp::messages::ocpp21::NotifyWebPaymentStartedConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::PublishFirmwareStatusNotificationReq,
                                                   ocpp::messages::ocpp21::PublishFirmwareStatusNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::PullDynamicScheduleUpdateReq,
                                                   ocpp::messages::ocpp21::PullDynamicScheduleUpdateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ReportChargingProfilesReq,
                                                   ocpp::messages::ocpp21::ReportChargingProfilesConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ReportDERControlReq,
                                                   ocpp::messages::ocpp21::ReportDERControlConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ReservationStatusUpdateReq,
                                                   ocpp::messages::ocpp21::ReservationStatusUpdateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SecurityEventNotificationReq,
                                                   ocpp::messages::ocpp21::SecurityEventNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::SignCertificateReq,
                                                   ocpp::messages::ocpp21::SignCertificateConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::StatusNotificationReq,
                                                   ocpp::messages::ocpp21::StatusNotificationConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::TransactionEventReq,
                                                   ocpp::messages::ocpp21::TransactionEventConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::VatNumberValidationReq,
                                                   ocpp::messages::ocpp21::VatNumberValidationConf>
{
  public:
    /**
     * @brief Constructor
     * @param identifier Charge point's identifier
     * @param messages_converter Converter from/to OCPP to/from JSON messages
     * @param msg_dispatcher Message dispatcher
     */
    ChargePointHandler21(const std::string&                               identifier,
                       const ocpp::messages::ocpp21::MessagesConverter21& messages_converter,
                       ocpp::messages::MessageDispatcher&               msg_dispatcher);
    /** @brief Destructor */
    virtual ~ChargePointHandler21();

    /** @brief Register the event handler */
    void registerHandler(IChargePointRequestHandler21& handler) { m_handler = &handler; }

    // OCPP handlers
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::AuthorizeReq& request,
                       ocpp::messages::ocpp21::AuthorizeConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::BatterySwapReq& request,
                       ocpp::messages::ocpp21::BatterySwapConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::BootNotificationReq& request,
                       ocpp::messages::ocpp21::BootNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::ClearedChargingLimitReq& request,
                       ocpp::messages::ocpp21::ClearedChargingLimitConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::CostUpdatedReq& request,
                       ocpp::messages::ocpp21::CostUpdatedConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::DataTransferReq& request,
                       ocpp::messages::ocpp21::DataTransferConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::FirmwareStatusNotificationReq& request,
                       ocpp::messages::ocpp21::FirmwareStatusNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::HeartbeatReq& request,
                       ocpp::messages::ocpp21::HeartbeatConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::LogStatusNotificationReq& request,
                       ocpp::messages::ocpp21::LogStatusNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::MeterValuesReq& request,
                       ocpp::messages::ocpp21::MeterValuesConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyAllowedEnergyTransferReq& request,
                       ocpp::messages::ocpp21::NotifyAllowedEnergyTransferConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyChargingLimitReq& request,
                       ocpp::messages::ocpp21::NotifyChargingLimitConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyCustomerInformationReq& request,
                       ocpp::messages::ocpp21::NotifyCustomerInformationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyDERAlarmReq& request,
                       ocpp::messages::ocpp21::NotifyDERAlarmConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyDERStartStopReq& request,
                       ocpp::messages::ocpp21::NotifyDERStartStopConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyDisplayMessagesReq& request,
                       ocpp::messages::ocpp21::NotifyDisplayMessagesConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyEVChargingNeedsReq& request,
                       ocpp::messages::ocpp21::NotifyEVChargingNeedsConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyEVChargingScheduleReq& request,
                       ocpp::messages::ocpp21::NotifyEVChargingScheduleConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyEventReq& request,
                       ocpp::messages::ocpp21::NotifyEventConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyMonitoringReportReq& request,
                       ocpp::messages::ocpp21::NotifyMonitoringReportConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyPeriodicEventStreamReq& request,
                       ocpp::messages::ocpp21::NotifyPeriodicEventStreamConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyPriorityChargingReq& request,
                       ocpp::messages::ocpp21::NotifyPriorityChargingConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyReportReq& request,
                       ocpp::messages::ocpp21::NotifyReportConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifySettlementReq& request,
                       ocpp::messages::ocpp21::NotifySettlementConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::NotifyWebPaymentStartedReq& request,
                       ocpp::messages::ocpp21::NotifyWebPaymentStartedConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::PublishFirmwareStatusNotificationReq& request,
                       ocpp::messages::ocpp21::PublishFirmwareStatusNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::PullDynamicScheduleUpdateReq& request,
                       ocpp::messages::ocpp21::PullDynamicScheduleUpdateConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::ReportChargingProfilesReq& request,
                       ocpp::messages::ocpp21::ReportChargingProfilesConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::ReportDERControlReq& request,
                       ocpp::messages::ocpp21::ReportDERControlConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::ReservationStatusUpdateReq& request,
                       ocpp::messages::ocpp21::ReservationStatusUpdateConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::SecurityEventNotificationReq& request,
                       ocpp::messages::ocpp21::SecurityEventNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::SignCertificateReq& request,
                       ocpp::messages::ocpp21::SignCertificateConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::StatusNotificationReq& request,
                       ocpp::messages::ocpp21::StatusNotificationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::TransactionEventReq& request,
                       ocpp::messages::ocpp21::TransactionEventConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;
    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp21::VatNumberValidationReq& request,
                       ocpp::messages::ocpp21::VatNumberValidationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

  private:
    /** @brief Charge point's identifier */
    const std::string m_identifier;
    /** @brief Request handler */
    IChargePointRequestHandler21* m_handler;
};

} // namespace ocpp21
} // namespace centralsystem
} // namespace ocpp

#endif // OPENOCPP_OCPP21_CS_CHARGEPOINTHANDLER21_H