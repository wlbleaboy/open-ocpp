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

#include "ChargePointConfig21.h"
#include "DefaultChargePointEventsHandler21.h"
#include "IChargePoint21.h"
#include "IniFile.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

using namespace ocpp::chargepoint::ocpp21;

/** @brief Entry point */
int main(int argc, char* argv[])
{
    std::string working_dir;
    bool        reset_all = false;

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

            argc--;
            argv++;
        }
        if (bad_param)
        {
            if (param)
            {
                std::cout << "Invalid parameter : " << param << std::endl;
            }
            std::cout << "Usage : quick_start_chargepoint21 [-w working_dir] [-r]" << std::endl;
            std::cout << "    -w : Working directory where to store the configuration file (Default = current directory)" << std::endl;
            std::cout << "    -r : Reset all the OCPP persistent data" << std::endl;
            return 1;
        }
    }

    std::cout << "Starting OCPP 2.1 charge point with :" << std::endl;
    std::cout << "  - working_dir = " << working_dir << std::endl;

    std::filesystem::path path(working_dir);
    path /= "quick_start_chargepoint21.ini";

    ocpp::helpers::IniFile config_file(path.string());
    ChargePointConfig21    stack_config(config_file);
    DefaultChargePointEventsHandler21 event_handler;

    std::unique_ptr<IChargePoint21> charge_point = IChargePoint21::create(stack_config, event_handler);
    event_handler.setChargePoint(*charge_point);
    if (reset_all)
    {
        charge_point->resetData();
    }

    if (!charge_point->start())
    {
        std::cout << "Unable to start OCPP 2.1 charge point" << std::endl;
        return 1;
    }

    std::cout << "OCPP 2.1 charge point started. Press ENTER to stop." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    charge_point->stop();
    return 0;
}
