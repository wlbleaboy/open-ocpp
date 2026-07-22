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

#include "Ocpp20MeterValueProvider.h"
#include "DateTime.h"

#include <algorithm>
#include <numeric>

using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

namespace
{

/** @brief Add a sampled value */
void addSampledValue(MeterValueType&         meter_value,
                     float                   value,
                     ReadingContextEnumType  context,
                     MeasurandEnumType       measurand,
                     const char*             unit,
                     Optional<PhaseEnumType> phase = Optional<PhaseEnumType>())
{
    SampledValueType sampled_value;
    sampled_value.value     = value;
    sampled_value.context   = context;
    sampled_value.measurand = measurand;
    sampled_value.location  = LocationEnumType::Outlet;
    if (phase.isSet())
    {
        sampled_value.phase = phase.value();
    }
    sampled_value.unitOfMeasure.value().unit.value().assign(unit);
    meter_value.sampledValue.push_back(sampled_value);
}

/** @brief Convert phase index to OCPP phase */
Optional<PhaseEnumType> phaseFromIndex(size_t index)
{
    Optional<PhaseEnumType> phase;
    switch (index)
    {
        case 0:
            phase = PhaseEnumType::L1;
            break;
        case 1:
            phase = PhaseEnumType::L2;
            break;
        case 2:
            phase = PhaseEnumType::L3;
            break;
        default:
            break;
    }
    return phase;
}

} // namespace

/** @brief Constructor */
Ocpp20MeterValueProvider::Ocpp20MeterValueProvider() : m_meters(), m_mutex() { }

/** @brief Destructor */
Ocpp20MeterValueProvider::~Ocpp20MeterValueProvider() { }

/** @brief Register a meter for an EVSE */
void Ocpp20MeterValueProvider::setMeter(unsigned int evse_id, IMeter& meter)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_meters[evse_id] = &meter;
}

/** @brief Unregister a meter for an EVSE */
void Ocpp20MeterValueProvider::removeMeter(unsigned int evse_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_meters.erase(evse_id);
}

/** @brief Fill a meter value sample */
bool Ocpp20MeterValueProvider::getMeterValue(unsigned int evse_id, ReadingContextEnumType context, MeterValueType& meter_value)
{
    IMeter* meter = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        iter = m_meters.find(evse_id);
        if (iter != m_meters.end())
        {
            meter = iter->second;
        }
    }

    if (meter == nullptr)
    {
        return false;
    }

    const int64_t                   energy_wh = meter->getEnergy();
    const std::vector<unsigned int> powers    = meter->getInstantPowers();
    const std::vector<unsigned int> currents  = meter->getCurrents();
    const std::vector<unsigned int> voltages  = meter->getVoltages();

    meter_value.timestamp = DateTime::now();
    meter_value.sampledValue.clear();

    addSampledValue(meter_value,
                    static_cast<float>(energy_wh),
                    context,
                    MeasurandEnumType::Energy_Active_Import_Register,
                    "Wh");

    const unsigned int total_power = std::accumulate(powers.begin(), powers.end(), 0u);
    addSampledValue(meter_value,
                    static_cast<float>(total_power),
                    context,
                    MeasurandEnumType::Power_Active_Import,
                    "W");

    const size_t phases_count = std::min<size_t>({static_cast<size_t>(meter->getNumberOfPhases()), currents.size(), voltages.size()});
    for (size_t i = 0; i < phases_count; i++)
    {
        const Optional<PhaseEnumType> phase = phaseFromIndex(i);
        addSampledValue(meter_value,
                        static_cast<float>(currents[i]),
                        context,
                        MeasurandEnumType::Current_Import,
                        "A",
                        phase);
        addSampledValue(meter_value,
                        static_cast<float>(voltages[i]),
                        context,
                        MeasurandEnumType::Voltage,
                        "V",
                        phase);
    }

    return true;
}
