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

#include "RequestFifoManager21.h"

#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "Logger.h"
#include "TransactionEvent21.h"
#include "WorkerThreadPool.h"

#include <functional>

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

RequestFifoManager21::RequestFifoManager21(IChargePointEventsHandler21&          events_handler,
                                           ocpp::helpers::ITimerPool&            timer_pool,
                                           ocpp::helpers::WorkerThreadPool&      worker_pool,
                                           ocpp::messages::GenericMessageSender& msg_sender,
                                           ocpp::messages::IRequestFifo&         requests_fifo)
    : m_events_handler(events_handler),
      m_worker_pool(worker_pool),
      m_msg_sender(msg_sender),
      m_requests_fifo(requests_fifo),
      m_request_retry_timer(timer_pool, "OCPP2.1 requests FIFO"),
      m_process_mutex()
{
    m_request_retry_timer.setCallback([this] { m_worker_pool.run<void>(std::bind(&RequestFifoManager21::processFifoRequest, this)); });
    m_requests_fifo.registerListener(this);
}

RequestFifoManager21::~RequestFifoManager21()
{
    m_requests_fifo.registerListener(nullptr);
}

void RequestFifoManager21::updateConnectionStatus(bool is_connected)
{
    if (is_connected && !m_requests_fifo.empty())
    {
        LOG_INFO << "Restart OCPP2.1 FIFO processing";
        m_worker_pool.run<void>(std::bind(&RequestFifoManager21::processFifoRequest, this));
    }
}

void RequestFifoManager21::requestQueued()
{
    if (m_msg_sender.isConnected() && !m_request_retry_timer.isStarted())
    {
        m_worker_pool.run<void>(std::bind(&RequestFifoManager21::processFifoRequest, this));
    }
}

void RequestFifoManager21::processFifoRequest()
{
    std::lock_guard<std::mutex> lock(m_process_mutex);

    while (m_msg_sender.isConnected() && !m_requests_fifo.empty() && !m_request_retry_timer.isStarted())
    {
        std::string         action;
        rapidjson::Document payload;
        unsigned int        connector_id = 0;
        if (m_requests_fifo.front(connector_id, action, payload))
        {
            (void)connector_id;
            CallResult result = CallResult::Failed;

            if (action == TRANSACTIONEVENT_ACTION)
            {
                TransactionEventConf response;
                result = m_msg_sender.call(action, payload, response);
            }
            else
            {
                LOG_WARNING << "Unsupported OCPP2.1 FIFO action [" << action << "], drop message";
                result = CallResult::Ok;
            }

            if (result == CallResult::Ok)
            {
                LOG_DEBUG << "OCPP2.1 FIFO request succeeded";
                m_requests_fifo.pop();
            }
            else
            {
                LOG_DEBUG << "OCPP2.1 FIFO request failed, next retry in " << RETRY_INTERVAL_SECONDS << " second(s)";
                m_request_retry_timer.restart(std::chrono::seconds(RETRY_INTERVAL_SECONDS), true);
            }
        }
        else
        {
            LOG_WARNING << "Invalid OCPP2.1 FIFO payload, drop message";
            m_requests_fifo.pop();
        }
    }
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
