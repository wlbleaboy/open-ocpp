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

#ifndef OPENOCPP_OCPP20_REQUESTFIFOMANAGER20_H
#define OPENOCPP_OCPP20_REQUESTFIFOMANAGER20_H

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
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Replay OCPP 2.0.1 offline requests */
class RequestFifoManager20 : public ocpp::messages::IRequestFifo::IListener
{
  public:
    /** @brief Constructor */
    RequestFifoManager20(IChargePointEventsHandler20&          events_handler,
                         ocpp::helpers::ITimerPool&            timer_pool,
                         ocpp::helpers::WorkerThreadPool&      worker_pool,
                         ocpp::messages::GenericMessageSender& msg_sender,
                         ocpp::messages::IRequestFifo&         requests_fifo);

    /** @brief Destructor */
    virtual ~RequestFifoManager20();

    /** @brief Update connection status */
    void updateConnectionStatus(bool is_connected);

    // IRequestFifo::IListener interface

    /** @copydoc void IRequestFifo::IListener::requestQueued() */
    void requestQueued() override;

  private:
    /** @brief Retry interval for failed offline requests */
    static constexpr unsigned int RETRY_INTERVAL_SECONDS = 5u;

    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Worker thread pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Request FIFO */
    ocpp::messages::IRequestFifo& m_requests_fifo;
    /** @brief Retry timer */
    ocpp::helpers::Timer m_request_retry_timer;
    /** @brief Mutex against concurrent processing */
    std::mutex m_process_mutex;

    /** @brief Process queued requests */
    void processFifoRequest();
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_REQUESTFIFOMANAGER20_H
