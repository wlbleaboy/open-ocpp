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

#include "ChargePointEventsHandler.h"
#include "ChargePointDemoConfig.h"
#include "IMeter.h"
#include "ISetpointManager.h"
#include "StringHelpers.h"

#include "tools.h"
#include "publics.h"

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;
using namespace ocpp::types;

/** @brief Constructor */
ChargePointEventsHandler::ChargePointEventsHandler(ChargePointDemoConfig& config, const std::filesystem::path& working_dir)
    : DefaultChargePointEventsHandler21(), m_meter_simulators(nullptr), m_setpoint_manager(nullptr),m_chargerManager(nullptr)
{
    (void)config;
    (void)working_dir;
}

/** @brief Destructor */
ChargePointEventsHandler::~ChargePointEventsHandler() { }

/** @copydoc int IChargePointEventsHandler::getTxStartStopMeterValue(unsigned int) */
int ChargePointEventsHandler::getTxStartStopMeterValue(unsigned int connector_id)
{
    int ret = 0;
    zlog_info(gZlog, "Get start/stop meter value for connector: %d", connector_id);

    if (m_meter_simulators)
    {
        IMeter* meter_simulator = (*m_meter_simulators)[connector_id].get();
#if ( PRJ_NAME == PRJ_BSL_1)
        ret = static_cast<int>(meter_simulator->getEnergy());
#elif ( PRJ_NAME == PRJ_JINLANG_1)
        ret = static_cast<int>(meter_simulator->getEnergy()) * 10; // Wh
#endif
        
    }
    return ret;
}

/** @copydoc void IChargePointEventsHandler::connectionStateChanged(bool) */
void ChargePointEventsHandler::connectionStateChanged(bool isConnected)
{    
    if(m_chargerManager)
    {
        m_chargerManager->connectionStateChanged(isConnected);
    }    
}

void ChargePointEventsHandler::connectionFailed(RegistrationStatus status)
{
    if(m_chargerManager)
    {
        m_chargerManager->connectionFailed(status);
    }  
}

ConfigurationStatus ChargePointEventsHandler::setConfiguration(const std::string& key, const std::string& value)
{
    zlog_info(gZlog, "setConfiguration : %s - %s", key.c_str(), value.c_str());
    ConfigurationStatus ret = ConfigurationStatus::Rejected;
    
    if(m_chargerManager)
    {
        ret = m_chargerManager->setConfiguration(key, value);
    }
    else
    {
        zlog_error(gZlog, "m_chargerManager is null");
    }

    return ret;
}

void ChargePointEventsHandler::sendLocalList(std::vector<std::string> localList)
{
    if(m_chargerManager)
    {
        m_chargerManager->sendLocalList(localList);
    }
    else
    {
        zlog_error(gZlog, "m_chargerManager is null");
    }
}

/** @copydoc void IChargePointEventsHandler::bootNotification(RegistrationStatus, const DateTime&) */
void ChargePointEventsHandler::bootNotification(RegistrationStatus status, const DateTime& datetime)
{
    zlog_info(gZlog, "Bootnotification : %s - %s", RegistrationStatusHelper.toString(status).c_str(), datetime.str().c_str());
    if(m_chargerManager)
    {
        m_chargerManager->bootNotification(status, datetime);
    }
}

/** @copydoc void IChargePointEventsHandler::datetimeReceived(const DateTime&) */
void ChargePointEventsHandler::datetimeReceived(const DateTime& datetime)
{   
    long ts = static_cast<long>(datetime.timestamp());
    long ts_now = DateTime::now().timestamp();
    std::string strNow = DateTime(ts_now).str();
    

    int diff = abs(static_cast<int>(ts_now - ts));

    if(diff > 60)
    {
        zlog_info(gZlog, "Date time [received : %s], [Local : %s], diff : %d seconds", datetime.str().c_str(), strNow.c_str(), diff);
        std::string strDatetime = datetime.str();
        strDatetime.replace(strDatetime.find("T"), 1, " ");
        std::string cmd = "date -s \"" + strDatetime + "\"";
        system(cmd.c_str());
        zlog_info(gZlog, "set system datetime to :%s cmd[%s]", strDatetime.c_str(), cmd.c_str());
    }    
}

/** @copydoc bool IChargePointEventsHandler::remoteStartTransactionRequested(unsigned int, const std::string&) */
bool ChargePointEventsHandler::remoteStartTransactionRequested(unsigned int connector_id, const std::string& id_tag)
{
    bool result = false;

    if(m_chargerManager)
    {
        result = m_chargerManager->remoteStartTransactionRequested(connector_id, id_tag);
    }

    return result;
}

/** @copydoc bool IChargePointEventsHandler::remoteStopTransactionRequested(unsigned int) */
bool ChargePointEventsHandler::remoteStopTransactionRequested(unsigned int connector_id)
{
    bool result = false;

    if(m_chargerManager)
    {
        result = m_chargerManager->remoteStopTransactionRequested(connector_id);
    }

    return result;
}
bool ChargePointEventsHandler::resetRequested(ResetType reset_type)
{
    bool result = false;

    if(m_chargerManager)
    {
        result = m_chargerManager->resetRequested(reset_type);
    }

    return result;
}
  
/** @copydoc bool getMeterValue(unsigned int, const std::pair<Measurand, Optional<Phase>>&, MeterValue&) */
bool ChargePointEventsHandler::getMeterValue(unsigned int connector_id,
                                             const std::pair<Measurand, Optional<Phase>>& measurand,
                                             MeterValue& meter_value)
{
    bool ret = false;

    // zlog_info(gZlog, "getMeterValue : %d - %s", connector_id, MeasurandHelper.toString(measurand.first).c_str());

    if (m_meter_simulators)
    {
        SampledValue value;
        IMeter*      meter_simulator = (*m_meter_simulators)[connector_id].get();
        ret                          = true;

        int l1n = int(Phase::L1N);

        switch (measurand.first)
        {
            case Measurand::CurrentImport:
            {
                auto currents = meter_simulator->getCurrents();
                if (measurand.second.isSet())
                {
                    unsigned int phase = static_cast<unsigned int>(measurand.second.value());
                    if (phase <= meter_simulator->getNumberOfPhases())
                    {
                        value.value = FormatFloat(currents[phase], 1);                        
                        value.location = Location::Outlet;
                        value.format = ValueFormat::Raw;
                        value.unit = UnitOfMeasure::A;
#if ( PRJ_NAME == PRJ_BSL_1)
                        value.context = ReadingContext::SamplePeriodic;
#elif ( PRJ_NAME == PRJ_JINLANG_1)

                        value.context = ReadingContext::SampleClock;
#endif
                        value.measurand = Measurand::CurrentOffered;
                        meter_value.sampledValue.push_back(value);
                    }
                    else
                    {
                        ret = false;
                    }
                }
                else
                {
                    for (size_t i = 0; i < currents.size(); i++)
                    {
                        value.value = FormatFloat(currents[i], 1);                        
                        value.location = Location::Outlet;
                        value.format = ValueFormat::Raw;
                        value.unit = UnitOfMeasure::A;
#if ( PRJ_NAME == PRJ_BSL_1)
                        value.context = ReadingContext::SamplePeriodic;
						value.phase = static_cast<Phase>(i);
#elif ( PRJ_NAME == PRJ_JINLANG_1)
                        value.context = ReadingContext::SampleClock;
						value.phase = static_cast<Phase>(l1n+i);
#endif                       
                        value.measurand = Measurand::CurrentOffered;                       
                        meter_value.sampledValue.push_back(value);
                    }
                }
            }
            break;

            case Measurand::CurrentOffered:
            {
                auto setpoints = m_setpoint_manager->getSetpoints();
                value.value    = std::to_string(static_cast<unsigned int>(setpoints[connector_id]));
                meter_value.sampledValue.push_back(value);
            }
            break;
           


            case Measurand::EnergyActiveImportRegister:
            {
                //本次充电电量
#if ( PRJ_NAME == PRJ_BSL_1)
                uint32_t energy = meter_simulator->getEnergy(); // Convert to Wh
                value.value = std::to_string(energy);   
#elif ( PRJ_NAME == PRJ_JINLANG_1)
                uint32_t energy = meter_simulator->getEnergies()[1] * 10;
                value.value = std::to_string(energy); 
#endif               
                value.location = Location::Outlet;
                value.format = ValueFormat::Raw;
                value.unit = UnitOfMeasure::Wh;
                value.context = ReadingContext::SampleClock;
                value.measurand = Measurand::EnergyActiveImportRegister;
                meter_value.sampledValue.push_back(value);
            }
            break;

#if ( PRJ_NAME == PRJ_JINLANG_1)		
			// 0-总电量，1-本次充电电量，2-今日总电量，3-本月总电量
			case Measurand::EnergyActiveExportRegister:
            {
                //电表累计电量
                uint32_t energy = meter_simulator->getEnergies()[0] * 10;
                value.value = std::to_string(energy);               
                value.location = Location::Outlet;
                value.format = ValueFormat::Raw;
                value.unit = UnitOfMeasure::Wh;
                value.context = ReadingContext::SampleClock;
                value.measurand = Measurand::EnergyActiveExportRegister;
                meter_value.sampledValue.push_back(value);
            }
            break;            

            // 0-总电量，1-本次充电电量，2-今日总电量，3-本月总电量
            case Measurand::EnergyReactiveExportRegister:
            {
                //日电量
                uint32_t energy = meter_simulator->getEnergies()[2] * 10;
                value.value = std::to_string(energy);                
                value.location = Location::Outlet;
                value.format = ValueFormat::Raw;
                value.unit = UnitOfMeasure::Wh;
                value.context = ReadingContext::SampleClock;
                value.measurand = Measurand::EnergyReactiveExportRegister;
                meter_value.sampledValue.push_back(value);
            }
            break;

            // 0-总电量，1-本次充电电量，2-今日总电量，3-本月总电量
            case Measurand::EnergyReactiveImportRegister:
            {
                //月电量
                uint32_t energy = meter_simulator->getEnergies()[3] * 10;
                value.value = std::to_string(energy);                 
                value.location = Location::Outlet;
                value.format = ValueFormat::Raw;
                value.unit = UnitOfMeasure::Wh;
                value.context = ReadingContext::SampleClock;
                value.measurand = Measurand::EnergyReactiveImportRegister;
                meter_value.sampledValue.push_back(value);
            }
            break;            
#endif 

            case Measurand::PowerActiveImport:
            {
                uint16_t power = meter_simulator->getPower();
                value.value = FormatFloat(power, 2);                        
                value.location = Location::Outlet;
                value.format = ValueFormat::Raw;
                value.unit = UnitOfMeasure::kW;
                value.context = ReadingContext::SampleClock;
                value.measurand = Measurand::PowerActiveImport;
                meter_value.sampledValue.push_back(value);
            }
            break;

            case Measurand::Voltage:
            {
                auto voltages = meter_simulator->getVoltages();
                if (measurand.second.isSet())
                {
                    unsigned int phase = static_cast<unsigned int>(measurand.second.value());
                    if (phase <= meter_simulator->getNumberOfPhases())
                    {
                        value.value = FormatFloat(voltages[phase], 1);
                        value.phase = static_cast<Phase>(phase);
                        meter_value.sampledValue.push_back(value);
                    }
                    else
                    {
                        ret = false;
                    }
                }
                else
                {
                    for (size_t i = 0; i < voltages.size(); i++)
                    {
                        value.value = FormatFloat(voltages[i], 1);                        
                        value.location = Location::Outlet;
                        value.format = ValueFormat::Raw;
                        value.unit = UnitOfMeasure::V;
                        value.context = ReadingContext::SampleClock;
                        value.measurand = Measurand::Voltage;
                        value.phase = static_cast<Phase>(l1n+i);
                        meter_value.sampledValue.push_back(value);
                    }
                }
            }
            break;

            case Measurand::Temperature:
            {
                auto temperaturesVector = meter_simulator->getTemperatures();
                std::string temperatures = "";

                for (size_t i = 0; i < temperaturesVector.size(); i++)
                {
                    int temp = static_cast<int>(temperaturesVector[i]);
                    temperatures += std::to_string(temp);
                    if (i != temperaturesVector.size() - 1)
                    {
                        temperatures += ",";
                    }                        
                }

                value.value = temperatures;
                value.context = ReadingContext::SampleClock;
                value.unit = UnitOfMeasure::Celsius;
                value.format = ValueFormat::Raw;
                meter_value.sampledValue.push_back(value);
            }
            break;

            default:
            {
                ret = false;
            }
            break;
        }
    }

    return ret;
}

/** @copydoc std::string IChargePointEventsHandler::getDiagnostics(const Optional<DateTime>&,
                                                                       const Optional<DateTime>&) */
std::string ChargePointEventsHandler::getDiagnostics(const Optional<DateTime>& start_time,
                                                            const Optional<DateTime>& stop_time)
{
    zlog_debug(gZlog, "Get diagnostics requested");
    zlog_debug(gZlog, "Start time: %s, Stop time: %s",
               start_time.isSet() ? start_time.value().str().c_str() : "Not set",
               stop_time.isSet() ? stop_time.value().str().c_str() : "Not set");

    if (start_time > stop_time)
    {
        zlog_error(gZlog, "getDiagnostics error: The start time is later than the end time");
        return "";
    }

    bool copied_any = false;
    time_t timestampNow = time(nullptr);
    std::time_t start = 0;
    std::time_t end = 0;

    std::string sn = m_chargerManager->getConfigureValue("ChargePoint","ChargePointSerialNumber").toString();    
    
    const std::filesystem::path dest_dir = "/tmp/diagnostics";
    const std::filesystem::path db_dir = "/home/data/database";
    std::filesystem::path diag_file = "/tmp/" + sn + "_diagnostics";
  
    if (!std::filesystem::exists(LOG_DIR) || !std::filesystem::is_directory(LOG_DIR))
    {
        zlog_error(gZlog, "%s not exist or is not a directory", LOG_DIR);
        return "";
    }

    if(start_time.isSet())
    {
        start = start_time.value().timestamp();
        diag_file += "_" + start_time.value().str();
    }
    else
    {
        start = 0;
        diag_file += "_start";
    }

    if(stop_time.isSet())
    {
        end = stop_time.value().timestamp();

        if(timestampNow - end < 60)
        {
            end += 60;
        }

        diag_file += "_" + stop_time.value().str();
    }
    else
    {
        end = timestampNow;
        diag_file += "_stop";
    }
    
    diag_file += ".zip";

    try {
        if (std::filesystem::exists(dest_dir)) 
        { 
            std::filesystem::remove_all(dest_dir);           
        }

        std::filesystem::create_directories(dest_dir);
        zlog_debug(gZlog, "create DIR: %s", dest_dir.string().c_str());

        for (const auto& entry : std::filesystem::recursive_directory_iterator(LOG_DIR)) 
        {
            if (!entry.is_regular_file()) 
            {
                continue;
            }

            std::string filename = entry.path().string();
            
            struct stat fileInfo;

            if (stat(filename.c_str(), &fileInfo) != 0) 
            {
                std::cerr << "Error: Cannot access file " << filename << std::endl;
                continue;
            }

            time_t modified_time = fileInfo.st_mtime;

            // 1356969600 // 2013-01-01 00:00:00 UTC
            if(modified_time < 1356969600 || (modified_time >= start && modified_time <= end) )
            {  
                std::filesystem::copy(entry.path(), dest_dir, std::filesystem::copy_options::overwrite_existing);
                copied_any = true;
            }
        }        
    } catch (const std::filesystem::filesystem_error& e) {
        zlog_error(gZlog, "getDiagnostics: File system error: %s", e.what());
        diag_file = "";
    } catch (const std::exception& e) {
        zlog_error(gZlog, "getDiagnostics: Unexpected error: %s", e.what());
        diag_file = "";
    }

    if (std::filesystem::exists(db_dir) && std::filesystem::is_directory(db_dir))
    {
        try {
            // 遍历源目录
            for (const auto& entry : std::filesystem::recursive_directory_iterator(db_dir)) {
                // 检查是否为文件且后缀为 .db
                if (entry.is_regular_file() && entry.path().extension() == ".db") {               
                    std::filesystem::copy(entry.path(), dest_dir, std::filesystem::copy_options::overwrite_existing);
                    copied_any = true;
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            zlog_error(gZlog, "getDiagnostics: File system error: %s", e.what());
        }        
    }
    
    if(copied_any)
    {
        std::stringstream ssZipCmd;
        ssZipCmd << "zip -r -q " << diag_file << " " << dest_dir;
        int err = WEXITSTATUS(system(ssZipCmd.str().c_str()));
        cout << "Command line : " << ssZipCmd.str() << " => " << err << endl;
    }
    else
    {
        zlog_warn(gZlog, "getDiagnostics: No log files found within the time range");
        diag_file = "";
    }   

    return diag_file;
}

/** @copydoc IChargePointEventsHandler21::onUpdateFirmware(...) */
bool ChargePointEventsHandler::onUpdateFirmware(const ocpp::messages::ocpp21::UpdateFirmwareReq& request,
                                                ocpp::messages::ocpp21::UpdateFirmwareConf&      response,
                                                std::string&                                     error,
                                                std::string&                                     message,
                                                std::string&                                     local_firmware_file)
{
    (void)error;
    (void)message;

    zlog_debug(gZlog, "Firmware update requested, requestId=%d, location=%s", request.requestId, request.firmware.location.str().c_str());
    if (access(TEMP_DIR, F_OK) != 0)
    {
        if (mkdir(TEMP_DIR, DIR_PERM) != 0)
        {
            zlog_error(gZlog, "create %s failed", TEMP_DIR);
            response.status = ocpp::types::ocpp21::UpdateFirmwareStatusEnumType::Rejected;
            return true;
        }
    }

    local_firmware_file = string(TEMP_DIR) + "/firmware.zip";
    response.status = ocpp::types::ocpp21::UpdateFirmwareStatusEnumType::Accepted;
    return true;
}

/** @copydoc IChargePointEventsHandler21::installFirmware(const std::string&) */
void ChargePointEventsHandler::installFirmware(const std::string& firmware_file)
{
    if (m_chargerManager)
    {
        m_chargerManager->installFirmware(firmware_file);
    }
    else
    {
        zlog_error(gZlog, "m_chargerManager is null");
    }
}

/** @copydoc DataTransferStatus IChargePointEventsHandler::dataTransferRequested(const std::string&,
                                                                                              const std::string&,
                                                                                              const std::string&,
                                                                                              std::string&) */
DataTransferStatus ChargePointEventsHandler::dataTransferRequested(const std::string& vendor_id,
                                                                                const std::string& message_id,
                                                                                const std::string& request_data,
                                                                                std::string&       response_data)
{
    DataTransferStatus ret = DataTransferStatus::Accepted;

    zlog_info(gZlog, "Data transfer received: vendor_id: %s, message_id: %s,request_data:%s", vendor_id.c_str(), message_id.c_str(),request_data.c_str());

    if( m_chargerManager )
    {
        ret = m_chargerManager->dataTransferRequested( vendor_id, message_id, request_data, response_data );
    }

    return ret;
}

UnlockStatus ChargePointEventsHandler::unlockConnectorRequested(unsigned int connector_id)
{
    UnlockStatus ret = UnlockStatus::NotSupported;

    zlog_info(gZlog, "unlockConnectorRequested received: connector_id: %d", connector_id);

    if( m_chargerManager )
    {
        ret = m_chargerManager->unlockConnectorRequested( connector_id );
    }

    return ret;
}

/** @copydoc AvailabilityStatus IChargePointEventsHandler::changeAvailabilityRequested(unsigned int, AvailabilityType) */
AvailabilityStatus ChargePointEventsHandler::changeAvailabilityRequested(unsigned int                  connector_id,
    AvailabilityType availability)
{
    AvailabilityStatus ret;

    zlog_info(gZlog, "changeAvailabilityRequested: connector_id: %d, availability: %s", connector_id, AvailabilityTypeHelper.toString(availability).c_str());
    if( m_chargerManager )
    {
        ret = m_chargerManager->changeAvailability( connector_id, availability );
    }else
    {
        zlog_error(gZlog, "m_chargerManager is null");
        ret = AvailabilityStatus::Rejected;
    }
    
    return ret;
}

 /** @copydoc void IChargePointEventsHandler::reservationStarted(unsigned int) */
void ChargePointEventsHandler::reservationStarted(unsigned int connector_id)
{
    zlog_info(gZlog, "reservationStarted received: connector_id: %d", connector_id);

    if(m_chargerManager)
    {
        m_chargerManager->changeReserveStatus(connector_id, 1);
    }
    else
    {
        zlog_error(gZlog, "m_chargerManager is null");
    }
}

/** @copydoc void IChargePointEventsHandler::reservationEnded(unsigned int, unsigned int) 
 * 
 * 
*/
void ChargePointEventsHandler::reservationEnded(unsigned int connector_id, unsigned int canceled)
{
    zlog_info(gZlog, "reservationEnded received: connector_id: %d, canceled: %d", connector_id, canceled);

    if(m_chargerManager)
    {
        m_chargerManager->changeReserveStatus(connector_id, canceled);
    }
    else
    {
        zlog_error(gZlog, "m_chargerManager is null");
    }
}
