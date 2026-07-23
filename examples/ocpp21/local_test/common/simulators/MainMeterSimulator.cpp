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

#include "MainMeterSimulator.h"

/** @brief Constructor */
MainMeterSimulator::MainMeterSimulator(std::vector<IMeter*>& child_meters)
    : m_child_meters(child_meters),
      m_phases_count(m_child_meters.empty() ? 0u : m_child_meters[0]->getNumberOfPhases()),
      m_voltages(m_phases_count),
      m_energies(),
      m_energy(0),
      m_temperatures(),
      m_power(0u)
{
}

/** @brief Destructor */
MainMeterSimulator::~MainMeterSimulator() { }

/** @brief Start the meter */
void MainMeterSimulator::start() { }

/** @brief Stop the meter */
void MainMeterSimulator::stop() { }

/** @brief Set the currents in A */
void MainMeterSimulator::setCurrents(const std::vector<unsigned int> currents)
{
    (void)currents;
}

/** @brief Set the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
void MainMeterSimulator::setEnergies(const std::vector<uint32_t> energies)
{
    m_energies = energies;
}

/** @brief Set the total energy in Wh */
void MainMeterSimulator::setEnergy(int64_t energy)
{
    m_energy = energy;
}

/** @brief Get the currents in A */
std::vector<unsigned int> MainMeterSimulator::getCurrents()
{
    std::vector<unsigned int> currents(m_phases_count);
    for (auto& meter : m_child_meters)
    {
        std::vector<unsigned int> child_currents = meter->getCurrents();
        for (std::size_t i = 0; i < child_currents.size(); i++)
        {
            currents[i] += child_currents[i];
        }
    }
    return currents;
}

/** @brief Get the instant powers in W */
std::vector<unsigned int> MainMeterSimulator::getInstantPowers()
{
    std::vector<unsigned int> powers(m_phases_count);
    for (auto& meter : m_child_meters)
    {
        std::vector<unsigned int> child_powers = meter->getInstantPowers();
        for (std::size_t i = 0; i < child_powers.size(); i++)
        {
            powers[i] += child_powers[i];
        }
    }
    return powers;
}

/** @brief Get the total energy in Wh */
int64_t MainMeterSimulator::getEnergy()
{
    int64_t energy = 0ll;
    for (auto& meter : m_child_meters)
    {
        energy += meter->getEnergy();
    }
    return energy;
}

/** @brief Get the [total energy],[current energy],[total_oneday],[total_onemonth] in Wh */
std::vector<uint32_t> MainMeterSimulator::getEnergies()
{
    return m_energies;
}

/** @brief Get the temperatures in Celsius */
std::vector<unsigned int> MainMeterSimulator::getTemperatures()
{
    return m_temperatures;
}

/** @brief Set the temperatures in Celsius */
void MainMeterSimulator::setTemperatures(std::vector<unsigned int> temperatures)
{
    m_temperatures = std::move(temperatures);
}

/** @brief Set the power in KW */
void MainMeterSimulator::setPower(uint16_t power)
{
    m_power = power;
}

/** @brief Get the power in KW */
uint16_t MainMeterSimulator::getPower()
{
    return m_power;
}
