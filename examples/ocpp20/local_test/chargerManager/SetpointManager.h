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

#ifndef SETPOINTMANAGER_H
#define SETPOINTMANAGER_H

#include "IChargePoint20.h"
#include "ISetpointManager.h"
#include "Timer.h"

#include <mutex>
#include <set>

/** @brief Manage the setpoints for the charge point and its connectors */
class SetpointManager : public ISetpointManager
{
  public:
    /** @brief Constructor */
    SetpointManager(ocpp::chargepoint::ocpp20::IChargePoint20& charge_point,
                    unsigned int                                evse_count,
                    unsigned int                     max_charge_point_current,
                    unsigned int                     max_evse_current);

    /** @brief Destructor */
    virtual ~SetpointManager();

    /** @brief Get the setpoints of each connectors */
    std::vector<float> getSetpoints() override;
    /** @brief Get the setpoints of a connectors */
    float getSetpoint(unsigned int evse_id) override;
    /** @brief Mark an EVSE as charging or idle */
    void setEvseCharging(unsigned int evse_id, bool charging) override;

  private:
    /** @brief Charge point */
    ocpp::chargepoint::ocpp20::IChargePoint20& m_charge_point;
    /** @brief Timer to update meter setpoints */
    ocpp::helpers::Timer m_update_timer;
    /** @brief Number of EVSEs */
    const unsigned int m_evse_count;
    /** @brief Max current that can handle the charge point */
    const unsigned int m_max_charge_point_current;
    /** @brief Max current that can handle an EVSE of the charge point*/
    const unsigned int m_max_evse_current;

    /** @brief Mutex to protect concurrent access to setpoints */
    std::mutex m_mutex;
    /** @brief Setpoints */
    std::vector<float> m_setpoints;
    /** @brief EVSEs with an ongoing transaction */
    std::set<unsigned int> m_charging_evses;

    /** @brief Update period */
    static constexpr std::chrono::milliseconds UPDATE_PERIOD = std::chrono::seconds(1u);

    /** @brief Periodically update the setpoints */
    void update();
};

#endif // SETPOINTMANAGER_H
