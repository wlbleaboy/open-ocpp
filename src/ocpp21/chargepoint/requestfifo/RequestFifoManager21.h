/*
Copyright (c) 2020 Cedric Jimenez
This file is part of OpenOCPP.

OpenOCPP is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

OpenOCPP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenOCPP. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENOCPP_OCPP21_REQUESTFIFOMANAGER21_H
#define OPENOCPP_OCPP21_REQUESTFIFOMANAGER21_H

#include "IRequestFifo.h"
#include "Timer.h"

#include <mutex>

namespace ocpp
{
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers
namespace messages
{
class GenericMessageSender;
} // namespace messages

namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Replay OCPP 2.1 offline requests */
class RequestFifoManager21 : public ocpp::messages::IRequestFifo::IListener
{
  public:
    RequestFifoManager21(IChargePointEventsHandler21&          events_handler,
                         ocpp::helpers::ITimerPool&            timer_pool,
                         ocpp::helpers::WorkerThreadPool&      worker_pool,
                         ocpp::messages::GenericMessageSender& msg_sender,
                         ocpp::messages::IRequestFifo&         requests_fifo);
    virtual ~RequestFifoManager21();

    void updateConnectionStatus(bool is_connected);
    void requestQueued() override;

  private:
    static constexpr unsigned int RETRY_INTERVAL_SECONDS = 5u;

    IChargePointEventsHandler21&          m_events_handler;
    ocpp::helpers::WorkerThreadPool&      m_worker_pool;
    ocpp::messages::GenericMessageSender& m_msg_sender;
    ocpp::messages::IRequestFifo&         m_requests_fifo;
    ocpp::helpers::Timer                  m_request_retry_timer;
    std::mutex                            m_process_mutex;

    void processFifoRequest();
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_REQUESTFIFOMANAGER21_H
