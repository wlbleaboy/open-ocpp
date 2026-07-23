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

#ifndef CHARGEPOINTEVENTSHANDLER_H
#define CHARGEPOINTEVENTSHANDLER_H

#include "DefaultChargePointEventsHandler21.h"
#include "IChargerManager.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class ChargePointDemoConfig;
class IMeter;
class ISetpointManager;
class IChargerManager;

using namespace ocpp::types;

/** @brief Charge point event handlers implementation */
class ChargePointEventsHandler : public DefaultChargePointEventsHandler21
{
  public:
    /** @brief Constructor */
    ChargePointEventsHandler(ChargePointDemoConfig& config, const std::filesystem::path& working_dir);

    /** @brief Destructor */
    virtual ~ChargePointEventsHandler();

    // IChargePointEventsHandler interface

    /** @copydoc int IChargePointEventsHandler::getTxStartStopMeterValue(unsigned int) */
    int getTxStartStopMeterValue(unsigned int connector_id);

    /** @copydoc bool IChargePointEventsHandler::getMeterValue(unsigned int, const std::pair<Measurand, Optional<Phase>>&, MeterValue&) */
    bool getMeterValue(unsigned int                                                                        connector_id,
                       const std::pair<Measurand, Optional<Phase>>& measurand,
                       MeterValue&                                                            meter_value);

    // API

    /** @brief Set the meter simulators */
    void setMeterSimulators(std::vector<std::unique_ptr<IMeter>>& meter_simulators) { m_meter_simulators = &meter_simulators; }

    /** @brief Set the setpoint manager */
    void setSetpointManager(ISetpointManager& setpoint_manager) { m_setpoint_manager = &setpoint_manager; }

    /** @copydoc void IChargePointEventsHandler::connectionStateChanged(bool) */
    void connectionStateChanged(bool isConnected) override;

    /** @copydoc void IChargePointEventsHandler::connectionStateChanged(RegistrationStatus) */
    void connectionFailed(RegistrationStatus status);

    ConfigurationStatus setConfiguration(const std::string& key, const std::string& value);

    void sendLocalList(std::vector<std::string> localList);

    /** @copydoc void IChargePointEventsHandler::bootNotification(RegistrationStatus, const DateTime&) */
    void bootNotification(RegistrationStatus status, const DateTime& datetime);

    /** @copydoc void IChargePointEventsHandler::datetimeReceived(const DateTime&) */
    void datetimeReceived(const DateTime& datetime);

    /** @copydoc bool IChargePointEventsHandler::remoteStartTransactionRequested(unsigned int, const std::string&) */
    bool remoteStartTransactionRequested(unsigned int connector_id, const std::string& id_tag);

    /** @copydoc bool IChargePointEventsHandler::remoteStopTransactionRequested(unsigned int) */
    bool remoteStopTransactionRequested(unsigned int connector_id);

    /** @copydoc DataTransferStatus IChargePointEventsHandler::dataTransferRequested(const std::string&,
                                                                                                  const std::string&,
                                                                                                  const std::string&,
                                                                                                  std::string&) */
    DataTransferStatus dataTransferRequested(const std::string& vendor_id,
                                                          const std::string& message_id,
                                                          const std::string& request_data,
                                                          std::string&       response_data);
                                                          
    UnlockStatus unlockConnectorRequested(unsigned int connector_id);

    /** @copydoc std::string IChargePointEventsHandler::getDiagnostics(const Optional<DateTime>&,
                                                                       const Optional<DateTime>&) */
    std::string getDiagnostics(const Optional<DateTime>& start_time,
                               const Optional<DateTime>& stop_time);

    /** @copydoc IChargePointEventsHandler21::onUpdateFirmware(...) */
    bool onUpdateFirmware(const ocpp::messages::ocpp21::UpdateFirmwareReq& request,
                          ocpp::messages::ocpp21::UpdateFirmwareConf&      response,
                          std::string&                                     error,
                          std::string&                                     message,
                          std::string&                                     local_firmware_file) override;

    /** @copydoc IChargePointEventsHandler21::installFirmware(const std::string&) */
    void installFirmware(const std::string& firmware_file) override;

    void setChargerManager(IChargerManager& ChargerManager) { m_chargerManager = &ChargerManager; }
    
    /** @copydoc bool IChargePointEventsHandler::resetRequested(ResetType) */
    bool resetRequested(ResetType reset_type);

    /** @copydoc AvailabilityStatus IChargePointEventsHandler::changeAvailabilityRequested(unsigned int, AvailabilityType) */
    AvailabilityStatus changeAvailabilityRequested(unsigned int                  connector_id,
      AvailabilityType availability);

     /** @copydoc void IChargePointEventsHandler::reservationStarted(unsigned int) */
    void reservationStarted(unsigned int connector_id);

    /** @copydoc void IChargePointEventsHandler::reservationEnded(unsigned int, bool) */
    void reservationEnded(unsigned int connector_id, unsigned int canceled);
    
  private:
    /** @brief Meter simulators */
    std::vector<std::unique_ptr<IMeter>>* m_meter_simulators;
    /** @brief Setpoint manager */
    ISetpointManager* m_setpoint_manager;
    
    IChargerManager* m_chargerManager;
   
 
};

#endif // CHARGEPOINTEVENTSHANDLER_H
