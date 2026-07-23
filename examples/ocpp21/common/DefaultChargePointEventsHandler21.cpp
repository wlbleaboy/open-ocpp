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

#include "DefaultChargePointEventsHandler21.h"

#include "IChargePoint21.h"
#include "TransactionManager21.h"
#include "StringHelpers.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _MSC_VER
#define WEXITSTATUS(x) (x)
#endif // _MSC_VER

using namespace ocpp::messages::ocpp21;
using namespace ocpp::types::ocpp21;

DefaultChargePointEventsHandler21::DefaultChargePointEventsHandler21() : m_is_connected(false), m_charge_point(nullptr)
{
}

DefaultChargePointEventsHandler21::~DefaultChargePointEventsHandler21()
{
}

void DefaultChargePointEventsHandler21::connectionFailed()
{
    std::cout << "Connection failed" << std::endl;
}

void DefaultChargePointEventsHandler21::connectionStateChanged(bool isConnected)
{
    m_is_connected = isConnected;
    std::cout << "Connection state changed : " << (isConnected ? "connected" : "disconnected") << std::endl;
}

bool DefaultChargePointEventsHandler21::getMeterValue(unsigned int                                  evse_id,
                                                      ocpp::types::ocpp21::ReadingContextEnumType context,
                                                      ocpp::types::ocpp21::MeterValueType&         meter_value)
{
    (void)evse_id;
    (void)context;
    (void)meter_value;
    return false;
}

bool DefaultChargePointEventsHandler21::unsupported(const std::string& action, std::string& error, std::string& message)
{
    std::cout << "Unsupported OCPP 2.1 request received : " << action << std::endl;
    error   = "NotSupported";
    message = "This OCPP 2.1 example does not implement business handling for this request";
    return false;
}

bool DefaultChargePointEventsHandler21::onAFRRSignal(const AFRRSignalReq& request,
                                                AFRRSignalConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("AFRRSignal", error, message);
}

bool DefaultChargePointEventsHandler21::onAdjustPeriodicEventStream(const AdjustPeriodicEventStreamReq& request,
                                                AdjustPeriodicEventStreamConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("AdjustPeriodicEventStream", error, message);
}

bool DefaultChargePointEventsHandler21::onCancelReservation(const CancelReservationReq& request,
                                                CancelReservationConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("CancelReservation", error, message);
}

bool DefaultChargePointEventsHandler21::onCertificateSigned(const CertificateSignedReq& request,
                                                CertificateSignedConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("CertificateSigned", error, message);
}

bool DefaultChargePointEventsHandler21::onChangeAvailability(const ChangeAvailabilityReq& request,
                                                ChangeAvailabilityConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)error;
    (void)message;
    response.status = ChangeAvailabilityStatusEnumType::Accepted;
    return true;
}

bool DefaultChargePointEventsHandler21::onChangeTransactionTariff(const ChangeTransactionTariffReq& request,
                                                ChangeTransactionTariffConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ChangeTransactionTariff", error, message);
}

bool DefaultChargePointEventsHandler21::onClearCache(const ClearCacheReq& request,
                                                ClearCacheConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearCache", error, message);
}

bool DefaultChargePointEventsHandler21::onClearChargingProfile(const ClearChargingProfileReq& request,
                                                ClearChargingProfileConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearChargingProfile", error, message);
}

bool DefaultChargePointEventsHandler21::onClearDERControl(const ClearDERControlReq& request,
                                                ClearDERControlConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearDERControl", error, message);
}

bool DefaultChargePointEventsHandler21::onClearDisplayMessage(const ClearDisplayMessageReq& request,
                                                ClearDisplayMessageConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearDisplayMessage", error, message);
}

bool DefaultChargePointEventsHandler21::onClearTariffs(const ClearTariffsReq& request,
                                                ClearTariffsConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearTariffs", error, message);
}

bool DefaultChargePointEventsHandler21::onClearVariableMonitoring(const ClearVariableMonitoringReq& request,
                                                ClearVariableMonitoringConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClearVariableMonitoring", error, message);
}

bool DefaultChargePointEventsHandler21::onClosePeriodicEventStream(const ClosePeriodicEventStreamReq& request,
                                                ClosePeriodicEventStreamConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ClosePeriodicEventStream", error, message);
}

bool DefaultChargePointEventsHandler21::onCustomerInformation(const CustomerInformationReq& request,
                                                CustomerInformationConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("CustomerInformation", error, message);
}

bool DefaultChargePointEventsHandler21::onDataTransfer(const DataTransferReq& request,
                                                DataTransferConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("DataTransfer", error, message);
}

bool DefaultChargePointEventsHandler21::onDeleteCertificate(const DeleteCertificateReq& request,
                                                DeleteCertificateConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("DeleteCertificate", error, message);
}

bool DefaultChargePointEventsHandler21::onGet15118EVCertificate(const Get15118EVCertificateReq& request,
                                                Get15118EVCertificateConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("Get15118EVCertificate", error, message);
}

bool DefaultChargePointEventsHandler21::onGetBaseReport(const GetBaseReportReq& request,
                                                GetBaseReportConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetBaseReport", error, message);
}

bool DefaultChargePointEventsHandler21::onGetCertificateChainStatus(const GetCertificateChainStatusReq& request,
                                                GetCertificateChainStatusConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetCertificateChainStatus", error, message);
}

bool DefaultChargePointEventsHandler21::onGetCertificateStatus(const GetCertificateStatusReq& request,
                                                GetCertificateStatusConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetCertificateStatus", error, message);
}

bool DefaultChargePointEventsHandler21::onGetChargingProfiles(const GetChargingProfilesReq& request,
                                                GetChargingProfilesConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)error;
    (void)message;
    response.status = GetChargingProfileStatusEnumType::NoProfiles;
    return true;
}

bool DefaultChargePointEventsHandler21::onGetCompositeSchedule(const GetCompositeScheduleReq& request,
                                                GetCompositeScheduleConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetCompositeSchedule", error, message);
}

bool DefaultChargePointEventsHandler21::onGetDERControl(const GetDERControlReq& request,
                                                GetDERControlConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetDERControl", error, message);
}

bool DefaultChargePointEventsHandler21::onGetDisplayMessages(const GetDisplayMessagesReq& request,
                                                GetDisplayMessagesConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetDisplayMessages", error, message);
}

bool DefaultChargePointEventsHandler21::onGetInstalledCertificateIds(const GetInstalledCertificateIdsReq& request,
                                                GetInstalledCertificateIdsConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetInstalledCertificateIds", error, message);
}

bool DefaultChargePointEventsHandler21::onGetLocalListVersion(const GetLocalListVersionReq& request,
                                                GetLocalListVersionConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetLocalListVersion", error, message);
}

bool DefaultChargePointEventsHandler21::onGetLog(const GetLogReq& request,
                                                GetLogConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetLog", error, message);
}

bool DefaultChargePointEventsHandler21::onGetMonitoringReport(const GetMonitoringReportReq& request,
                                                GetMonitoringReportConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetMonitoringReport", error, message);
}

bool DefaultChargePointEventsHandler21::onGetPeriodicEventStream(const GetPeriodicEventStreamReq& request,
                                                GetPeriodicEventStreamConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetPeriodicEventStream", error, message);
}

bool DefaultChargePointEventsHandler21::onGetReport(const GetReportReq& request,
                                                GetReportConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetReport", error, message);
}

bool DefaultChargePointEventsHandler21::onGetTariffs(const GetTariffsReq& request,
                                                GetTariffsConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetTariffs", error, message);
}

bool DefaultChargePointEventsHandler21::onGetTransactionStatus(const GetTransactionStatusReq& request,
                                                GetTransactionStatusConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetTransactionStatus", error, message);
}

bool DefaultChargePointEventsHandler21::onGetVariables(const GetVariablesReq& request,
                                                GetVariablesConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("GetVariables", error, message);
}

bool DefaultChargePointEventsHandler21::onInstallCertificate(const InstallCertificateReq& request,
                                                InstallCertificateConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("InstallCertificate", error, message);
}

bool DefaultChargePointEventsHandler21::onOpenPeriodicEventStream(const OpenPeriodicEventStreamReq& request,
                                                OpenPeriodicEventStreamConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("OpenPeriodicEventStream", error, message);
}

bool DefaultChargePointEventsHandler21::onPublishFirmware(const PublishFirmwareReq& request,
                                                PublishFirmwareConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("PublishFirmware", error, message);
}

bool DefaultChargePointEventsHandler21::onRequestBatterySwap(const RequestBatterySwapReq& request,
                                                RequestBatterySwapConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("RequestBatterySwap", error, message);
}

bool DefaultChargePointEventsHandler21::onRequestStartTransaction(const RequestStartTransactionReq& request,
                                                RequestStartTransactionConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)error;
    (void)message;

    const unsigned int evse_id = static_cast<unsigned int>(request.evseId.isSet() ? request.evseId.value() : 1);
    if (m_charge_point && evse_id > 0)
    {
        response.status = RequestStartStopStatusEnumType::Accepted;
        std::cout << "RequestStartTransaction accepted : evse = " << evse_id
                  << " remoteStartId = " << request.remoteStartId
                  << " token = " << request.idToken.idToken.str() << std::endl;

        auto* charge_point = m_charge_point;
        auto  id_token = request.idToken;
        const int remote_start_id = request.remoteStartId;
        std::thread([charge_point, evse_id, id_token, remote_start_id]() {
            std::string transaction_id;
            const bool started = charge_point->startTransaction(evse_id,
                                                                1u,
                                                                id_token,
                                                                TriggerReasonEnumType::RemoteStart,
                                                                remote_start_id,
                                                                transaction_id);
            if (!started)
            {
                std::cout << "Unable to start transaction from RequestStartTransaction : evse = " << evse_id << std::endl;
            }
            else
            {
                std::cout << "Transaction started from RequestStartTransaction : " << transaction_id << std::endl;
            }
        }).detach();
    }
    else
    {
        response.status = RequestStartStopStatusEnumType::Rejected;
        std::cout << "RequestStartTransaction rejected : invalid charge point or EVSE id " << evse_id << std::endl;
    }
    return true;
}

bool DefaultChargePointEventsHandler21::onRequestStopTransaction(const RequestStopTransactionReq& request,
                                                RequestStopTransactionConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)error;
    (void)message;

    const std::string transaction_id = request.transactionId.str();
    if (m_charge_point && !transaction_id.empty() && m_charge_point->hasActiveTransaction(transaction_id))
    {
        response.status = RequestStartStopStatusEnumType::Accepted;
        std::cout << "RequestStopTransaction accepted : " << transaction_id << std::endl;

        auto* charge_point = m_charge_point;
        std::thread([charge_point, transaction_id]() {
            const bool stopped = charge_point->stopTransaction(transaction_id,
                                                              ReasonEnumType::Remote,
                                                              TriggerReasonEnumType::RemoteStop,
                                                              nullptr,
                                                              {});
            if (!stopped)
            {
                std::cout << "Unable to stop transaction from RequestStopTransaction : " << transaction_id << std::endl;
            }
        }).detach();
    }
    else
    {
        response.status = RequestStartStopStatusEnumType::Rejected;
        std::cout << "RequestStopTransaction rejected : unknown or inactive transaction id " << transaction_id << std::endl;
    }
    return true;
}

bool DefaultChargePointEventsHandler21::onReserveNow(const ReserveNowReq& request,
                                                ReserveNowConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("ReserveNow", error, message);
}

bool DefaultChargePointEventsHandler21::onReset(const ResetReq& request,
                                                ResetConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("Reset", error, message);
}

bool DefaultChargePointEventsHandler21::onSendLocalList(const SendLocalListReq& request,
                                                SendLocalListConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SendLocalList", error, message);
}

bool DefaultChargePointEventsHandler21::onSetChargingProfile(const SetChargingProfileReq& request,
                                                SetChargingProfileConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetChargingProfile", error, message);
}

bool DefaultChargePointEventsHandler21::onSetDefaultTariff(const SetDefaultTariffReq& request,
                                                SetDefaultTariffConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetDefaultTariff", error, message);
}

bool DefaultChargePointEventsHandler21::onSetDERControl(const SetDERControlReq& request,
                                                SetDERControlConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetDERControl", error, message);
}

bool DefaultChargePointEventsHandler21::onSetDisplayMessage(const SetDisplayMessageReq& request,
                                                SetDisplayMessageConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetDisplayMessage", error, message);
}

bool DefaultChargePointEventsHandler21::onSetMonitoringBase(const SetMonitoringBaseReq& request,
                                                SetMonitoringBaseConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetMonitoringBase", error, message);
}

bool DefaultChargePointEventsHandler21::onSetMonitoringLevel(const SetMonitoringLevelReq& request,
                                                SetMonitoringLevelConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetMonitoringLevel", error, message);
}

bool DefaultChargePointEventsHandler21::onSetNetworkProfile(const SetNetworkProfileReq& request,
                                                SetNetworkProfileConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetNetworkProfile", error, message);
}

bool DefaultChargePointEventsHandler21::onSetVariableMonitoring(const SetVariableMonitoringReq& request,
                                                SetVariableMonitoringConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetVariableMonitoring", error, message);
}

bool DefaultChargePointEventsHandler21::onSetVariables(const SetVariablesReq& request,
                                                SetVariablesConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("SetVariables", error, message);
}

bool DefaultChargePointEventsHandler21::onTriggerMessage(const TriggerMessageReq& request,
                                                TriggerMessageConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("TriggerMessage", error, message);
}

bool DefaultChargePointEventsHandler21::onUnlockConnector(const UnlockConnectorReq& request,
                                                UnlockConnectorConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)error;
    (void)message;

    std::cout << "UnlockConnector : evseId = " << request.evseId << " - connectorId = " << request.connectorId << std::endl;
    if ((request.evseId <= 0) || (request.connectorId <= 0))
    {
        response.status = UnlockStatusEnumType::UnknownConnector;
    }
    else if (m_charge_point && m_charge_point->getTransactionManager().hasActiveTransaction(
                                  static_cast<unsigned int>(request.evseId), static_cast<unsigned int>(request.connectorId)))
    {
        response.status = UnlockStatusEnumType::OngoingAuthorizedTransaction;
    }
    else
    {
        response.status = UnlockStatusEnumType::Unlocked;
    }
    return true;
}

bool DefaultChargePointEventsHandler21::onUnpublishFirmware(const UnpublishFirmwareReq& request,
                                                UnpublishFirmwareConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("UnpublishFirmware", error, message);
}

bool DefaultChargePointEventsHandler21::onUpdateDynamicSchedule(const UpdateDynamicScheduleReq& request,
                                                UpdateDynamicScheduleConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("UpdateDynamicSchedule", error, message);
}

bool DefaultChargePointEventsHandler21::onUpdateFirmware(const UpdateFirmwareReq& request,
                                                UpdateFirmwareConf&      response,
                                                std::string&   error,
                                                std::string&   message,
                                                std::string&   local_firmware_file)
{
    (void)error;
    (void)message;

    std::cout << "UpdateFirmware accepted : requestId = " << request.requestId
              << " - location = " << request.firmware.location.str() << std::endl;

    try
    {
        const std::filesystem::path firmware_dir = std::filesystem::temp_directory_path();
        std::filesystem::create_directories(firmware_dir);
        local_firmware_file = (firmware_dir / "firmware21.bin").string();
        response.status     = UpdateFirmwareStatusEnumType::Accepted;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cout << "Unable to prepare firmware download directory : " << e.what() << std::endl;
        response.status = UpdateFirmwareStatusEnumType::Rejected;
    }
    return true;
}

bool DefaultChargePointEventsHandler21::downloadFile(const std::string& url, const std::string& file)
{
    bool ret = true;
    std::cout << "Downloading from " << url << " to " << file << std::endl;

    std::string connection_url = url;
    std::string params;
    if (connection_url.find("ftp://") == 0)
    {
    }
    else if (connection_url.find("ftps://") == 0)
    {
        params = "--insecure --ssl";
        ocpp::helpers::replace(connection_url, "ftps://", "ftp://", false);
    }
    else if (connection_url.find("http://") == 0)
    {
    }
    else if (connection_url.find("https://") == 0)
    {
        params = "--insecure";
    }
    else
    {
        ret = false;
    }
    if (ret)
    {
        std::stringstream ss;
        ss << "curl --silent " << params << " -o " << file << " " << connection_url;
        int sys_ret = system(ss.str().c_str());
        int err     = WEXITSTATUS(sys_ret);
        std::cout << "Command line : " << ss.str() << std::endl;
        ret = (err == 0);
    }
    return ret;
}

void DefaultChargePointEventsHandler21::installFirmware(const std::string& firmware_file)
{
    std::cout << "Firmware to install : " << firmware_file << std::endl;
}

bool DefaultChargePointEventsHandler21::onUsePriorityCharging(const UsePriorityChargingReq& request,
                                                UsePriorityChargingConf&      response,
                                                std::string&   error,
                                                std::string&   message)
{
    (void)request;
    (void)response;
    return unsupported("UsePriorityCharging", error, message);
}
