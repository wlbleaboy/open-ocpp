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

#include "ChargePointDemoConfig.h"
#include "DefaultChargePointEventsHandler.h"
#include "DeviceModelManager20.h"
#include "IChargePoint20.h"
#include "MeterSimulator.h"
#include "Ocpp20MeterValueProvider.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <thread>

using namespace ocpp::chargepoint::ocpp20;
using namespace ocpp::types;
using namespace ocpp::types::ocpp20;
using namespace ocpp::messages::ocpp20;

/** @brief Entry point */
int main(int argc, char* argv[])
{
    // Default parameters
    std::string id_tag      = "0123456789ABCD";
    std::string working_dir = "";
    bool        reset_all   = false;

    // Check parameters
    if (argc > 1)
    {
        const char* param     = nullptr;
        bool        bad_param = false;
        argv++;
        while ((argc != 1) && !bad_param)
        {
            if (strcmp(*argv, "-h") == 0)
            {
                bad_param = true;
            }
            else if ((strcmp(*argv, "-t") == 0) && (argc > 1))
            {
                argv++;
                argc--;
                id_tag = *argv;
            }
            else if ((strcmp(*argv, "-w") == 0) && (argc > 1))
            {
                argv++;
                argc--;
                working_dir = *argv;
            }
            else if (strcmp(*argv, "-r") == 0)
            {
                reset_all = true;
            }
            else
            {
                param     = *argv;
                bad_param = true;
            }

            // Next param
            argc--;
            argv++;
        }
        if (bad_param)
        {
            if (param)
            {
                std::cout << "Invalid parameter : " << param << std::endl;
            }
            std::cout << "Usage : quick_start_chargepoint20 [-t id_tag] [-w working_dir] [-r]" << std::endl;
            std::cout << "    -t : Id token to use (Default = 0123456789ABCD)" << std::endl;
            std::cout << "    -w : Working directory where to store the configuration file (Default = current directory)" << std::endl;
            std::cout << "    -r : Reset all the OCPP persistent data" << std::endl;
            return 1;
        }
    }

    std::cout << "Starting charge point with :" << std::endl;
    std::cout << "  - id_token = " << id_tag << std::endl;
    std::cout << "  - working_dir = " << working_dir << std::endl;

    // Configuration
    std::filesystem::path path(working_dir);
    path /= "quick_start_chargepoint20.ini";
    ChargePointDemoConfig config(path.string());

    // Device model
    std::filesystem::path device_model_path(working_dir);
    device_model_path /= "quick_start_chargepoint20.json";
    DeviceModelManager device_model_mgr(config.stackConfig());
    if (device_model_mgr.init())
    {
        if (!device_model_mgr.load(device_model_path))
        {
            std::cout << "Unable to load device model : " << device_model_mgr.lastError() << std::endl;
        }
    }
    else
    {
        std::cout << "Unable to initialize device model loader : " << device_model_mgr.lastError() << std::endl;
    }

    // Event handler
    DefaultChargePointEventsHandler event_handler(config, device_model_mgr, working_dir);

    // Instanciate charge point
    std::unique_ptr<IChargePoint20> charge_point = IChargePoint20::create(config.stackConfig(), device_model_mgr, event_handler);
    if (reset_all)
    {
        charge_point->resetData();
    }

    MeterSimulator evse1_meter(charge_point->getTimerPool(), 3u);
    evse1_meter.setVoltages({230u, 230u, 230u});
    evse1_meter.setCurrents({10u, 10u, 10u});
    evse1_meter.start();

    MeterSimulator evse2_meter(charge_point->getTimerPool(), 3u);
    evse2_meter.setVoltages({230u, 230u, 230u});
    evse2_meter.setCurrents({6u, 6u, 6u});
    evse2_meter.start();

    Ocpp20MeterValueProvider meter_value_provider;
    meter_value_provider.setMeter(1u, evse1_meter);
    meter_value_provider.setMeter(2u, evse2_meter);

    event_handler.setMeterValueProvider(meter_value_provider);
    event_handler.setChargePoint(*charge_point.get());
    charge_point->start();

    // From now on the stack is alive :)

    while (true)
    {
        // Wait to be connected to the Central System
        std::cout << "Waiting connection to Central System..." << std::endl;
        while (!event_handler.isConnected())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100u));
        }
        std::cout << "Connected to Central System!" << std::endl;

        // Wait to be accepted by Central System
        std::cout << "Waiting registration to Central System..." << std::endl;
        while (event_handler.isConnected() && !event_handler.isRegistered())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100u));
        }
        if (!event_handler.isConnected())
        {
            std::cout << "Disconnected from Central System before registration" << std::endl;
            continue;
        }
        std::cout << "Registered to Central System!" << std::endl;

        // Enable station-level periodic MeterValues
        charge_point->startPeriodicMeterValues(1u, std::chrono::seconds(30u));

        // Test loop
        while (event_handler.isConnected())
        {
            for (unsigned int evse_id = 1u; evse_id <= 2u; evse_id++)
            {
                for (unsigned int connector_id = 1u; connector_id <= 3u; connector_id++)
                {
                    IdTokenType id_token;
                    id_token.idToken.assign(id_tag);
                    id_token.type = IdTokenEnumType::ISO14443;

                    std::cout << "Starting transaction on EVSE " << evse_id << ", connector " << connector_id << "..." << std::endl;

                    charge_point->statusNotification(evse_id, connector_id, ConnectorStatusEnumType::Occupied);
                    std::this_thread::sleep_for(std::chrono::seconds(1u));

                    std::string transaction_id;
                    if (charge_point->startTransaction(evse_id, connector_id, id_token, TriggerReasonEnumType::Authorized, transaction_id))
                    {
                        std::cout << "Transaction started : " << transaction_id << std::endl;

                        charge_point->updateTransaction(transaction_id, TriggerReasonEnumType::CablePluggedIn, {});
                        std::this_thread::sleep_for(std::chrono::seconds(30u));

                        if (charge_point->stopTransaction(transaction_id, ReasonEnumType::Local, TriggerReasonEnumType::StopAuthorized, &id_token, {}))
                        {
                            std::cout << "Transaction stopped : " << transaction_id << std::endl;
                        }
                        else
                        {
                            std::cout << "Unable to stop transaction : " << transaction_id << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Transaction rejected or delayed before start" << std::endl;
                    }

                    charge_point->statusNotification(evse_id, connector_id, ConnectorStatusEnumType::Available);

                    if (event_handler.isConnected())
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(10u));
                    }
                    else
                    {
                        std::cout << "Disconnected from Central System, stop test loop" << std::endl;
                        break;
                    }
                }
            }

            std::cout << "Test loop ended, wait before next cycle..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5u));
        }

        charge_point->stopPeriodicMeterValues(1u);
        std::cout << "Disconnected from Central System, wait for reconnection..." << std::endl;
    }

    return 0;
}
