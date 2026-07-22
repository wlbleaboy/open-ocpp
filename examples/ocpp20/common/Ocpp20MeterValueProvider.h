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

#ifndef OCPP20METERVALUEPROVIDER_H
#define OCPP20METERVALUEPROVIDER_H

#include "IMeter.h"
#include "MeterValueType20.h"
#include "ReadingContextEnumType20.h"

#include <map>
#include <mutex>

/** @brief Adapter from common meter simulators to OCPP 2.0.1 MeterValues */
class Ocpp20MeterValueProvider
{
  public:
    /** @brief Constructor */
    Ocpp20MeterValueProvider();

    /** @brief Destructor */
    virtual ~Ocpp20MeterValueProvider();

    /** @brief Register a meter for an EVSE */
    void setMeter(unsigned int evse_id, IMeter& meter);

    /** @brief Unregister a meter for an EVSE */
    void removeMeter(unsigned int evse_id);

    /** @brief Fill a meter value sample */
    bool getMeterValue(unsigned int                                                       evse_id,
                       ocpp::types::ocpp20::ReadingContextEnumType                       context,
                       ocpp::types::ocpp20::MeterValueType&                               meter_value);

  private:
    /** @brief Registered EVSE meters */
    std::map<unsigned int, IMeter*> m_meters;

    /** @brief Lock to protect meter registration */
    std::mutex m_mutex;
};

#endif // OCPP20METERVALUEPROVIDER_H
