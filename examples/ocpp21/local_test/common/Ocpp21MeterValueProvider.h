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

#ifndef OCPP21METERVALUEPROVIDER_H
#define OCPP21METERVALUEPROVIDER_H

#include "IMeter.h"
#include "MeterValueType21.h"
#include "ReadingContextEnumType21.h"

#include <map>
#include <mutex>

/** @brief Adapter from common meter simulators to OCPP 2.1 MeterValues */
class Ocpp21MeterValueProvider
{
  public:
    Ocpp21MeterValueProvider();
    virtual ~Ocpp21MeterValueProvider();

    void setMeter(unsigned int evse_id, IMeter& meter);
    void removeMeter(unsigned int evse_id);

    bool getMeterValue(unsigned int                                evse_id,
                       ocpp::types::ocpp21::ReadingContextEnumType context,
                       ocpp::types::ocpp21::MeterValueType&         meter_value);

  private:
    std::map<unsigned int, IMeter*> m_meters;
    std::mutex                      m_mutex;
};

#endif // OCPP21METERVALUEPROVIDER_H
