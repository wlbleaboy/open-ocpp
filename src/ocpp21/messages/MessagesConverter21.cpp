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

#include "MessagesConverter21.h"
#include "AFRRSignal21.h"
#include "AdjustPeriodicEventStream21.h"
#include "CancelReservation21.h"
#include "CertificateSigned21.h"
#include "ChangeAvailability21.h"
#include "ChangeTransactionTariff21.h"
#include "ClearCache21.h"
#include "ClearChargingProfile21.h"
#include "ClearDERControl21.h"
#include "ClearDisplayMessage21.h"
#include "ClearTariffs21.h"
#include "ClearVariableMonitoring21.h"
#include "ClosePeriodicEventStream21.h"
#include "CustomerInformation21.h"
#include "DataTransfer21.h"
#include "DeleteCertificate21.h"
#include "Get15118EVCertificate21.h"
#include "GetBaseReport21.h"
#include "GetCertificateChainStatus21.h"
#include "GetCertificateStatus21.h"
#include "GetChargingProfiles21.h"
#include "GetCompositeSchedule21.h"
#include "GetDERControl21.h"
#include "GetDisplayMessages21.h"
#include "GetInstalledCertificateIds21.h"
#include "GetLocalListVersion21.h"
#include "GetLog21.h"
#include "GetMonitoringReport21.h"
#include "GetPeriodicEventStream21.h"
#include "GetReport21.h"
#include "GetTariffs21.h"
#include "GetTransactionStatus21.h"
#include "GetVariables21.h"
#include "InstallCertificate21.h"
#include "OpenPeriodicEventStream21.h"
#include "PublishFirmware21.h"
#include "RequestBatterySwap21.h"
#include "RequestStartTransaction21.h"
#include "RequestStopTransaction21.h"
#include "ReserveNow21.h"
#include "Reset21.h"
#include "SendLocalList21.h"
#include "SetChargingProfile21.h"
#include "SetDefaultTariff21.h"
#include "SetDERControl21.h"
#include "SetDisplayMessage21.h"
#include "SetMonitoringBase21.h"
#include "SetMonitoringLevel21.h"
#include "SetNetworkProfile21.h"
#include "SetVariableMonitoring21.h"
#include "SetVariables21.h"
#include "TriggerMessage21.h"
#include "UnlockConnector21.h"
#include "UnpublishFirmware21.h"
#include "UpdateDynamicSchedule21.h"
#include "UpdateFirmware21.h"
#include "UsePriorityCharging21.h"
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

/** @brief Macro to register a message converter for an OCPP action */
#define REGISTER_CONVERTER(action)                                               \
    registerRequestConverter<action##Req>(#action, *new action##ReqConverter()); \
    registerResponseConverter<action##Conf>(#action, *new action##ConfConverter())

/** @brief Macro to delete a message converter for an OCPP action */
#define DELETE_CONVERTER(action)                  \
    deleteRequestConverter<action##Req>(#action); \
    deleteResponseConverter<action##Conf>(#action)

namespace ocpp
{
namespace messages
{
namespace ocpp21
{

/** @brief Constructor */
MessagesConverter21::MessagesConverter21()
{
    // Register converters
    REGISTER_CONVERTER(AFRRSignal);
    REGISTER_CONVERTER(AdjustPeriodicEventStream);
    REGISTER_CONVERTER(CancelReservation);
    REGISTER_CONVERTER(CertificateSigned);
    REGISTER_CONVERTER(ChangeAvailability);
    REGISTER_CONVERTER(ChangeTransactionTariff);
    REGISTER_CONVERTER(ClearCache);
    REGISTER_CONVERTER(ClearChargingProfile);
    REGISTER_CONVERTER(ClearDERControl);
    REGISTER_CONVERTER(ClearDisplayMessage);
    REGISTER_CONVERTER(ClearTariffs);
    REGISTER_CONVERTER(ClearVariableMonitoring);
    REGISTER_CONVERTER(ClosePeriodicEventStream);
    REGISTER_CONVERTER(CustomerInformation);
    REGISTER_CONVERTER(DataTransfer);
    REGISTER_CONVERTER(DeleteCertificate);
    REGISTER_CONVERTER(Get15118EVCertificate);
    REGISTER_CONVERTER(GetBaseReport);
    REGISTER_CONVERTER(GetCertificateChainStatus);
    REGISTER_CONVERTER(GetCertificateStatus);
    REGISTER_CONVERTER(GetChargingProfiles);
    REGISTER_CONVERTER(GetCompositeSchedule);
    REGISTER_CONVERTER(GetDERControl);
    REGISTER_CONVERTER(GetDisplayMessages);
    REGISTER_CONVERTER(GetInstalledCertificateIds);
    REGISTER_CONVERTER(GetLocalListVersion);
    REGISTER_CONVERTER(GetLog);
    REGISTER_CONVERTER(GetMonitoringReport);
    REGISTER_CONVERTER(GetPeriodicEventStream);
    REGISTER_CONVERTER(GetReport);
    REGISTER_CONVERTER(GetTariffs);
    REGISTER_CONVERTER(GetTransactionStatus);
    REGISTER_CONVERTER(GetVariables);
    REGISTER_CONVERTER(InstallCertificate);
    REGISTER_CONVERTER(OpenPeriodicEventStream);
    REGISTER_CONVERTER(PublishFirmware);
    REGISTER_CONVERTER(RequestBatterySwap);
    REGISTER_CONVERTER(RequestStartTransaction);
    REGISTER_CONVERTER(RequestStopTransaction);
    REGISTER_CONVERTER(ReserveNow);
    REGISTER_CONVERTER(Reset);
    REGISTER_CONVERTER(SendLocalList);
    REGISTER_CONVERTER(SetChargingProfile);
    REGISTER_CONVERTER(SetDefaultTariff);
    REGISTER_CONVERTER(SetDERControl);
    REGISTER_CONVERTER(SetDisplayMessage);
    REGISTER_CONVERTER(SetMonitoringBase);
    REGISTER_CONVERTER(SetMonitoringLevel);
    REGISTER_CONVERTER(SetNetworkProfile);
    REGISTER_CONVERTER(SetVariableMonitoring);
    REGISTER_CONVERTER(SetVariables);
    REGISTER_CONVERTER(TriggerMessage);
    REGISTER_CONVERTER(UnlockConnector);
    REGISTER_CONVERTER(UnpublishFirmware);
    REGISTER_CONVERTER(UpdateDynamicSchedule);
    REGISTER_CONVERTER(UpdateFirmware);
    REGISTER_CONVERTER(UsePriorityCharging);
    REGISTER_CONVERTER(Authorize);
    REGISTER_CONVERTER(BatterySwap);
    REGISTER_CONVERTER(BootNotification);
    REGISTER_CONVERTER(ClearedChargingLimit);
    REGISTER_CONVERTER(CostUpdated);
    REGISTER_CONVERTER(DataTransfer);
    REGISTER_CONVERTER(FirmwareStatusNotification);
    REGISTER_CONVERTER(Heartbeat);
    REGISTER_CONVERTER(LogStatusNotification);
    REGISTER_CONVERTER(MeterValues);
    REGISTER_CONVERTER(NotifyAllowedEnergyTransfer);
    REGISTER_CONVERTER(NotifyChargingLimit);
    REGISTER_CONVERTER(NotifyCustomerInformation);
    REGISTER_CONVERTER(NotifyDERAlarm);
    REGISTER_CONVERTER(NotifyDERStartStop);
    REGISTER_CONVERTER(NotifyDisplayMessages);
    REGISTER_CONVERTER(NotifyEVChargingNeeds);
    REGISTER_CONVERTER(NotifyEVChargingSchedule);
    REGISTER_CONVERTER(NotifyEvent);
    REGISTER_CONVERTER(NotifyMonitoringReport);
    REGISTER_CONVERTER(NotifyPeriodicEventStream);
    REGISTER_CONVERTER(NotifyPriorityCharging);
    REGISTER_CONVERTER(NotifyReport);
    REGISTER_CONVERTER(NotifySettlement);
    REGISTER_CONVERTER(NotifyWebPaymentStarted);
    REGISTER_CONVERTER(PublishFirmwareStatusNotification);
    REGISTER_CONVERTER(PullDynamicScheduleUpdate);
    REGISTER_CONVERTER(ReportChargingProfiles);
    REGISTER_CONVERTER(ReportDERControl);
    REGISTER_CONVERTER(ReservationStatusUpdate);
    REGISTER_CONVERTER(SecurityEventNotification);
    REGISTER_CONVERTER(SignCertificate);
    REGISTER_CONVERTER(StatusNotification);
    REGISTER_CONVERTER(TransactionEvent);
    REGISTER_CONVERTER(VatNumberValidation);
}
/** @brief Destructor */
MessagesConverter21::~MessagesConverter21()
{
    // Free memory
    DELETE_CONVERTER(AFRRSignal);
    DELETE_CONVERTER(AdjustPeriodicEventStream);
    DELETE_CONVERTER(CancelReservation);
    DELETE_CONVERTER(CertificateSigned);
    DELETE_CONVERTER(ChangeAvailability);
    DELETE_CONVERTER(ChangeTransactionTariff);
    DELETE_CONVERTER(ClearCache);
    DELETE_CONVERTER(ClearChargingProfile);
    DELETE_CONVERTER(ClearDERControl);
    DELETE_CONVERTER(ClearDisplayMessage);
    DELETE_CONVERTER(ClearTariffs);
    DELETE_CONVERTER(ClearVariableMonitoring);
    DELETE_CONVERTER(ClosePeriodicEventStream);
    DELETE_CONVERTER(CustomerInformation);
    DELETE_CONVERTER(DataTransfer);
    DELETE_CONVERTER(DeleteCertificate);
    DELETE_CONVERTER(Get15118EVCertificate);
    DELETE_CONVERTER(GetBaseReport);
    DELETE_CONVERTER(GetCertificateChainStatus);
    DELETE_CONVERTER(GetCertificateStatus);
    DELETE_CONVERTER(GetChargingProfiles);
    DELETE_CONVERTER(GetCompositeSchedule);
    DELETE_CONVERTER(GetDERControl);
    DELETE_CONVERTER(GetDisplayMessages);
    DELETE_CONVERTER(GetInstalledCertificateIds);
    DELETE_CONVERTER(GetLocalListVersion);
    DELETE_CONVERTER(GetLog);
    DELETE_CONVERTER(GetMonitoringReport);
    DELETE_CONVERTER(GetPeriodicEventStream);
    DELETE_CONVERTER(GetReport);
    DELETE_CONVERTER(GetTariffs);
    DELETE_CONVERTER(GetTransactionStatus);
    DELETE_CONVERTER(GetVariables);
    DELETE_CONVERTER(InstallCertificate);
    DELETE_CONVERTER(OpenPeriodicEventStream);
    DELETE_CONVERTER(PublishFirmware);
    DELETE_CONVERTER(RequestBatterySwap);
    DELETE_CONVERTER(RequestStartTransaction);
    DELETE_CONVERTER(RequestStopTransaction);
    DELETE_CONVERTER(ReserveNow);
    DELETE_CONVERTER(Reset);
    DELETE_CONVERTER(SendLocalList);
    DELETE_CONVERTER(SetChargingProfile);
    DELETE_CONVERTER(SetDefaultTariff);
    DELETE_CONVERTER(SetDERControl);
    DELETE_CONVERTER(SetDisplayMessage);
    DELETE_CONVERTER(SetMonitoringBase);
    DELETE_CONVERTER(SetMonitoringLevel);
    DELETE_CONVERTER(SetNetworkProfile);
    DELETE_CONVERTER(SetVariableMonitoring);
    DELETE_CONVERTER(SetVariables);
    DELETE_CONVERTER(TriggerMessage);
    DELETE_CONVERTER(UnlockConnector);
    DELETE_CONVERTER(UnpublishFirmware);
    DELETE_CONVERTER(UpdateDynamicSchedule);
    DELETE_CONVERTER(UpdateFirmware);
    DELETE_CONVERTER(UsePriorityCharging);
    DELETE_CONVERTER(Authorize);
    DELETE_CONVERTER(BatterySwap);
    DELETE_CONVERTER(BootNotification);
    DELETE_CONVERTER(ClearedChargingLimit);
    DELETE_CONVERTER(CostUpdated);
    DELETE_CONVERTER(DataTransfer);
    DELETE_CONVERTER(FirmwareStatusNotification);
    DELETE_CONVERTER(Heartbeat);
    DELETE_CONVERTER(LogStatusNotification);
    DELETE_CONVERTER(MeterValues);
    DELETE_CONVERTER(NotifyAllowedEnergyTransfer);
    DELETE_CONVERTER(NotifyChargingLimit);
    DELETE_CONVERTER(NotifyCustomerInformation);
    DELETE_CONVERTER(NotifyDERAlarm);
    DELETE_CONVERTER(NotifyDERStartStop);
    DELETE_CONVERTER(NotifyDisplayMessages);
    DELETE_CONVERTER(NotifyEVChargingNeeds);
    DELETE_CONVERTER(NotifyEVChargingSchedule);
    DELETE_CONVERTER(NotifyEvent);
    DELETE_CONVERTER(NotifyMonitoringReport);
    DELETE_CONVERTER(NotifyPeriodicEventStream);
    DELETE_CONVERTER(NotifyPriorityCharging);
    DELETE_CONVERTER(NotifyReport);
    DELETE_CONVERTER(NotifySettlement);
    DELETE_CONVERTER(NotifyWebPaymentStarted);
    DELETE_CONVERTER(PublishFirmwareStatusNotification);
    DELETE_CONVERTER(PullDynamicScheduleUpdate);
    DELETE_CONVERTER(ReportChargingProfiles);
    DELETE_CONVERTER(ReportDERControl);
    DELETE_CONVERTER(ReservationStatusUpdate);
    DELETE_CONVERTER(SecurityEventNotification);
    DELETE_CONVERTER(SignCertificate);
    DELETE_CONVERTER(StatusNotification);
    DELETE_CONVERTER(TransactionEvent);
    DELETE_CONVERTER(VatNumberValidation);
}

} // namespace ocpp21
} // namespace messages
} // namespace ocpp