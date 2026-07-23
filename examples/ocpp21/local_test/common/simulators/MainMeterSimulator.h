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

#ifndef MAINMETERSIMULATOR_H
#define MAINMETERSIMULATOR_H

#include "IMeter.h"

/** @brief Simulate a main meter which has multiple childs meters and its current/voltage consumption */
class MainMeterSimulator : public IMeter
{
  public:
    /** @brief Constructor */
    MainMeterSimulator(std::vector<IMeter*>& child_meters);

    /** @brief Destructor */
    virtual ~MainMeterSimulator();

    /** @brief Start the meter */
    void start() override;

    /** @brief Stop the meter */
    void stop() override;

    /** @brief Set the voltages in V */
    void setVoltages(const std::vector<unsigned int> voltages) override { m_voltages = voltages; }

    /** @brief Set the currents in A */
    void setCurrents(const std::vector<unsigned int> currents) override;

    /** @brief Set the total energy in Wh */
    void setEnergy(int64_t energy) override;

    /** @brief Set the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
    void setEnergies(const std::vector<uint32_t> energies) override;

    /** @brief Get the number of phases */
    unsigned int getNumberOfPhases() override { return m_phases_count; }

    /** @brief Get the voltages in V */
    std::vector<unsigned int> getVoltages() override { return m_voltages; }

    /** @brief Get the currents in A */
    std::vector<unsigned int> getCurrents() override;

    /** @brief Get the instant powers in W */
    std::vector<unsigned int> getInstantPowers() override;

    /** @brief Get the total energy in Wh */
    int64_t getEnergy() override;

    /** @brief Get the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
    std::vector<uint32_t> getEnergies() override;

    /** @brief Get the temperatures in Celsius */
    std::vector<unsigned int> getTemperatures() override;

    /** @brief Set the temperatures in Celsius */
    void setTemperatures(std::vector<unsigned int> temperatures) override;

    /** @brief Set the power in KW */
    void setPower(uint16_t power) override;

    /** @brief Get the power in KW */
    uint16_t getPower() override;

  private:
    /** @brief Child meters */
    std::vector<IMeter*>& m_child_meters;
    /** @brief Number of phases */
    const unsigned int m_phases_count;
    /** @brief Voltages in V */
    std::vector<unsigned int> m_voltages;
    /** @brief Instant [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
    std::vector<uint32_t> m_energies;
    /** @brief Total energy in Wh */
    int64_t m_energy;
    /** @brief Temperatures in Celsius */
    std::vector<unsigned int> m_temperatures;
    /** @brief Instant powers in KW */
    uint16_t m_power;
};

#endif // MAINMETERSIMULATOR_H
