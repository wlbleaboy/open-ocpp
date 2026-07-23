/*
MIT License

Copyright (c) 2020 Cedric Jimenez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef DEFAULTCHARGEPOINTEVENTSHANDLER21_H
#define DEFAULTCHARGEPOINTEVENTSHANDLER21_H

#include "IChargePointEventsHandler21.h"

#include <atomic>
#include <string>

class Ocpp21MeterValueProvider;
namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{
class IChargePoint21;
}
} // namespace chargepoint
} // namespace ocpp

/** @brief Default OCPP 2.1 charge point event handler for examples */
class DefaultChargePointEventsHandler21 : public ocpp::chargepoint::ocpp21::IChargePointEventsHandler21
{
  public:
    DefaultChargePointEventsHandler21();
    virtual ~DefaultChargePointEventsHandler21();

    bool isConnected() const { return m_is_connected.load(); }
    bool isRegistered() const { return m_is_registered.load(); }
    void setChargePoint(ocpp::chargepoint::ocpp21::IChargePoint21& charge_point) { m_charge_point = &charge_point; }
    void setMeterValueProvider(Ocpp21MeterValueProvider& meter_value_provider) { m_meter_value_provider = &meter_value_provider; }

    void connectionFailed() override;
    void connectionStateChanged(bool isConnected) override;
    bool getMeterValue(unsigned int                                  evse_id,
                       ocpp::types::ocpp21::ReadingContextEnumType context,
                       ocpp::types::ocpp21::MeterValueType&         meter_value) override;
    void bootNotification(ocpp::types::ocpp21::RegistrationStatusEnumType status,
                          const ocpp::types::DateTime&                    datetime) override;

    bool onAFRRSignal(const ocpp::messages::ocpp21::AFRRSignalReq& request,
                  ocpp::messages::ocpp21::AFRRSignalConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onAdjustPeriodicEventStream(const ocpp::messages::ocpp21::AdjustPeriodicEventStreamReq& request,
                  ocpp::messages::ocpp21::AdjustPeriodicEventStreamConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onCancelReservation(const ocpp::messages::ocpp21::CancelReservationReq& request,
                  ocpp::messages::ocpp21::CancelReservationConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onCertificateSigned(const ocpp::messages::ocpp21::CertificateSignedReq& request,
                  ocpp::messages::ocpp21::CertificateSignedConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onChangeAvailability(const ocpp::messages::ocpp21::ChangeAvailabilityReq& request,
                  ocpp::messages::ocpp21::ChangeAvailabilityConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onChangeTransactionTariff(const ocpp::messages::ocpp21::ChangeTransactionTariffReq& request,
                  ocpp::messages::ocpp21::ChangeTransactionTariffConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearCache(const ocpp::messages::ocpp21::ClearCacheReq& request,
                  ocpp::messages::ocpp21::ClearCacheConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearChargingProfile(const ocpp::messages::ocpp21::ClearChargingProfileReq& request,
                  ocpp::messages::ocpp21::ClearChargingProfileConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearDERControl(const ocpp::messages::ocpp21::ClearDERControlReq& request,
                  ocpp::messages::ocpp21::ClearDERControlConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearDisplayMessage(const ocpp::messages::ocpp21::ClearDisplayMessageReq& request,
                  ocpp::messages::ocpp21::ClearDisplayMessageConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearTariffs(const ocpp::messages::ocpp21::ClearTariffsReq& request,
                  ocpp::messages::ocpp21::ClearTariffsConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClearVariableMonitoring(const ocpp::messages::ocpp21::ClearVariableMonitoringReq& request,
                  ocpp::messages::ocpp21::ClearVariableMonitoringConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onClosePeriodicEventStream(const ocpp::messages::ocpp21::ClosePeriodicEventStreamReq& request,
                  ocpp::messages::ocpp21::ClosePeriodicEventStreamConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onCustomerInformation(const ocpp::messages::ocpp21::CustomerInformationReq& request,
                  ocpp::messages::ocpp21::CustomerInformationConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onDataTransfer(const ocpp::messages::ocpp21::DataTransferReq& request,
                  ocpp::messages::ocpp21::DataTransferConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onDeleteCertificate(const ocpp::messages::ocpp21::DeleteCertificateReq& request,
                  ocpp::messages::ocpp21::DeleteCertificateConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGet15118EVCertificate(const ocpp::messages::ocpp21::Get15118EVCertificateReq& request,
                  ocpp::messages::ocpp21::Get15118EVCertificateConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetBaseReport(const ocpp::messages::ocpp21::GetBaseReportReq& request,
                  ocpp::messages::ocpp21::GetBaseReportConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetCertificateChainStatus(const ocpp::messages::ocpp21::GetCertificateChainStatusReq& request,
                  ocpp::messages::ocpp21::GetCertificateChainStatusConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetCertificateStatus(const ocpp::messages::ocpp21::GetCertificateStatusReq& request,
                  ocpp::messages::ocpp21::GetCertificateStatusConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetChargingProfiles(const ocpp::messages::ocpp21::GetChargingProfilesReq& request,
                  ocpp::messages::ocpp21::GetChargingProfilesConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetCompositeSchedule(const ocpp::messages::ocpp21::GetCompositeScheduleReq& request,
                  ocpp::messages::ocpp21::GetCompositeScheduleConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetDERControl(const ocpp::messages::ocpp21::GetDERControlReq& request,
                  ocpp::messages::ocpp21::GetDERControlConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetDisplayMessages(const ocpp::messages::ocpp21::GetDisplayMessagesReq& request,
                  ocpp::messages::ocpp21::GetDisplayMessagesConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetInstalledCertificateIds(const ocpp::messages::ocpp21::GetInstalledCertificateIdsReq& request,
                  ocpp::messages::ocpp21::GetInstalledCertificateIdsConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetLocalListVersion(const ocpp::messages::ocpp21::GetLocalListVersionReq& request,
                  ocpp::messages::ocpp21::GetLocalListVersionConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetLog(const ocpp::messages::ocpp21::GetLogReq& request,
                  ocpp::messages::ocpp21::GetLogConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetMonitoringReport(const ocpp::messages::ocpp21::GetMonitoringReportReq& request,
                  ocpp::messages::ocpp21::GetMonitoringReportConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetPeriodicEventStream(const ocpp::messages::ocpp21::GetPeriodicEventStreamReq& request,
                  ocpp::messages::ocpp21::GetPeriodicEventStreamConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetReport(const ocpp::messages::ocpp21::GetReportReq& request,
                  ocpp::messages::ocpp21::GetReportConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetTariffs(const ocpp::messages::ocpp21::GetTariffsReq& request,
                  ocpp::messages::ocpp21::GetTariffsConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetTransactionStatus(const ocpp::messages::ocpp21::GetTransactionStatusReq& request,
                  ocpp::messages::ocpp21::GetTransactionStatusConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onGetVariables(const ocpp::messages::ocpp21::GetVariablesReq& request,
                  ocpp::messages::ocpp21::GetVariablesConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onInstallCertificate(const ocpp::messages::ocpp21::InstallCertificateReq& request,
                  ocpp::messages::ocpp21::InstallCertificateConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onOpenPeriodicEventStream(const ocpp::messages::ocpp21::OpenPeriodicEventStreamReq& request,
                  ocpp::messages::ocpp21::OpenPeriodicEventStreamConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onPublishFirmware(const ocpp::messages::ocpp21::PublishFirmwareReq& request,
                  ocpp::messages::ocpp21::PublishFirmwareConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onRequestBatterySwap(const ocpp::messages::ocpp21::RequestBatterySwapReq& request,
                  ocpp::messages::ocpp21::RequestBatterySwapConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onRequestStartTransaction(const ocpp::messages::ocpp21::RequestStartTransactionReq& request,
                  ocpp::messages::ocpp21::RequestStartTransactionConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onRequestStopTransaction(const ocpp::messages::ocpp21::RequestStopTransactionReq& request,
                  ocpp::messages::ocpp21::RequestStopTransactionConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onReserveNow(const ocpp::messages::ocpp21::ReserveNowReq& request,
                  ocpp::messages::ocpp21::ReserveNowConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onReset(const ocpp::messages::ocpp21::ResetReq& request,
                  ocpp::messages::ocpp21::ResetConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSendLocalList(const ocpp::messages::ocpp21::SendLocalListReq& request,
                  ocpp::messages::ocpp21::SendLocalListConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetChargingProfile(const ocpp::messages::ocpp21::SetChargingProfileReq& request,
                  ocpp::messages::ocpp21::SetChargingProfileConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetDefaultTariff(const ocpp::messages::ocpp21::SetDefaultTariffReq& request,
                  ocpp::messages::ocpp21::SetDefaultTariffConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetDERControl(const ocpp::messages::ocpp21::SetDERControlReq& request,
                  ocpp::messages::ocpp21::SetDERControlConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetDisplayMessage(const ocpp::messages::ocpp21::SetDisplayMessageReq& request,
                  ocpp::messages::ocpp21::SetDisplayMessageConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetMonitoringBase(const ocpp::messages::ocpp21::SetMonitoringBaseReq& request,
                  ocpp::messages::ocpp21::SetMonitoringBaseConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetMonitoringLevel(const ocpp::messages::ocpp21::SetMonitoringLevelReq& request,
                  ocpp::messages::ocpp21::SetMonitoringLevelConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetNetworkProfile(const ocpp::messages::ocpp21::SetNetworkProfileReq& request,
                  ocpp::messages::ocpp21::SetNetworkProfileConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetVariableMonitoring(const ocpp::messages::ocpp21::SetVariableMonitoringReq& request,
                  ocpp::messages::ocpp21::SetVariableMonitoringConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onSetVariables(const ocpp::messages::ocpp21::SetVariablesReq& request,
                  ocpp::messages::ocpp21::SetVariablesConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onTriggerMessage(const ocpp::messages::ocpp21::TriggerMessageReq& request,
                  ocpp::messages::ocpp21::TriggerMessageConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onUnlockConnector(const ocpp::messages::ocpp21::UnlockConnectorReq& request,
                  ocpp::messages::ocpp21::UnlockConnectorConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onUnpublishFirmware(const ocpp::messages::ocpp21::UnpublishFirmwareReq& request,
                  ocpp::messages::ocpp21::UnpublishFirmwareConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onUpdateDynamicSchedule(const ocpp::messages::ocpp21::UpdateDynamicScheduleReq& request,
                  ocpp::messages::ocpp21::UpdateDynamicScheduleConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;
    bool onUpdateFirmware(const ocpp::messages::ocpp21::UpdateFirmwareReq& request,
                  ocpp::messages::ocpp21::UpdateFirmwareConf&      response,
                  std::string&                            error,
                  std::string&                            message,
                  std::string&                            local_firmware_file) override;
    bool downloadFile(const std::string& url, const std::string& file) override;
    void installFirmware(const std::string& firmware_file) override;
    bool onUsePriorityCharging(const ocpp::messages::ocpp21::UsePriorityChargingReq& request,
                  ocpp::messages::ocpp21::UsePriorityChargingConf&      response,
                  std::string&                            error,
                  std::string&                            message) override;

  private:
    bool unsupported(const std::string& action, std::string& error, std::string& message);

    std::atomic_bool m_is_connected;
    std::atomic_bool m_is_registered;
    ocpp::chargepoint::ocpp21::IChargePoint21* m_charge_point;
    Ocpp21MeterValueProvider* m_meter_value_provider;
};

#endif // DEFAULTCHARGEPOINTEVENTSHANDLER21_H
