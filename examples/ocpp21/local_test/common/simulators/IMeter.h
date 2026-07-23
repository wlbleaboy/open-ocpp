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

#ifndef IMETER_H
#define IMETER_H

#include <cstdint>
#include <vector>

/** @brief Interface for meter simulators */
class IMeter
{
  public:
    /** @brief Destructor */
    virtual ~IMeter() { }

    /** @brief Start the meter */
    virtual void start() = 0;

    /** @brief Stop the meter */
    virtual void stop() = 0;

    /** @brief Set the voltages in V */
    virtual void setVoltages(const std::vector<unsigned int> voltages) = 0;

    /** @brief Set the currents in A */
    virtual void setCurrents(const std::vector<unsigned int> currents) = 0;

    /** @brief Set the total energy in Wh */
    virtual void setEnergy(int64_t energy) = 0;

    /** @brief Set the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
    virtual void setEnergies(const std::vector<uint32_t> energies) = 0;

    /** @brief Get the number of phases */
    virtual unsigned int getNumberOfPhases() = 0;

    /** @brief Get the voltages in V */
    virtual std::vector<unsigned int> getVoltages() = 0;

    /** @brief Get the currents in A */
    virtual std::vector<unsigned int> getCurrents() = 0;

    /** @brief Get the instant powers in W */
    virtual std::vector<unsigned int> getInstantPowers() = 0;

    /** @brief Get the total energy in Wh */
    virtual int64_t getEnergy() = 0;

    /** @brief Get the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
    virtual std::vector<uint32_t> getEnergies() = 0;

    /** @brief Get the temperatures in Celsius */
    virtual std::vector<unsigned int> getTemperatures() = 0;

    /** @brief Set the temperatures in Celsius */
    virtual void setTemperatures(std::vector<unsigned int> temperatures) = 0;

    /** @brief Set the power in KW */
    virtual void setPower(uint16_t power) = 0;

    /** @brief Get the power in KW */
    virtual uint16_t getPower() = 0;
};

#endif // IMETER_H
