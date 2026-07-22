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

#include "DataTransferManager20.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler20.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp20;
using namespace ocpp::types;
using namespace ocpp::types::ocpp20;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Constructor */
DataTransferManager20::DataTransferManager20(IChargePointEventsHandler20&                    events_handler,
                                             const ocpp::messages::GenericMessagesConverter& messages_converter,
                                             ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                             ocpp::messages::GenericMessageSender&           msg_sender)
    : GenericMessageHandler<DataTransferReq, DataTransferConf>(DATATRANSFER_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_handlers()
{
    msg_dispatcher.registerHandler(DATATRANSFER_ACTION, *this);
}

/** @brief Destructor */
DataTransferManager20::~DataTransferManager20() { }

/** @brief Send a data transfer request */
bool DataTransferManager20::dataTransfer(const std::string&                               vendor_id,
                                         const std::string&                               message_id,
                                         const std::string&                               request_data,
                                         ocpp::types::ocpp20::DataTransferStatusEnumType& status,
                                         std::string&                                     response_data,
                                         std::string&                                     error,
                                         std::string&                                     message)
{
    bool ret = false;

    // Fill request
    DataTransferReq request;
    request.vendorId.assign(vendor_id);
    if (!message_id.empty())
    {
        request.messageId.value().assign(message_id);
    }
    if (!request_data.empty())
    {
        request.data = request_data;
    }

    // Send request
    DataTransferConf response;
    if (m_msg_sender.call(DATATRANSFER_ACTION, request, response, error, message) == CallResult::Ok)
    {
        // Extract response
        status = response.status;
        response_data.clear();
        if (response.data.isSet())
        {
            response_data = response.data.value();
        }
        ret = true;
    }

    return ret;
}

/** @copydoc void IDataTransferManager20::registerHandler(const std::string&, IDataTransferHandler&) */
void DataTransferManager20::registerHandler(const std::string& vendor_id, IDataTransferHandler& handler)
{
    m_handlers[vendor_id] = &handler;
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
 *                                                                                ResponseType& response,
 *                                                                                std::string& error_code,
 *                                                                                std::string& error_message)
 */
bool DataTransferManager20::handleMessage(const ocpp::messages::ocpp20::DataTransferReq& request,
                                          ocpp::messages::ocpp20::DataTransferConf&      response,
                                          std::string&                                   error_code,
                                          std::string&                                   error_message)
{
    bool ret = true;

    // Check if a handler has been registered
    auto handler = m_handlers.find(request.vendorId.str());
    if (handler != m_handlers.cend())
    {
        std::string response_data;
        response.status = handler->second->onDataTransferRequest(request.vendorId.str(),
                                                                 request.messageId.isSet() ? request.messageId.value().str() : "",
                                                                 request.data.isSet() ? request.data.value() : "",
                                                                 response_data);
        if (!response_data.empty())
        {
            response.data = response_data;
        }
    }
    else
    {
        // Keep compatibility with the existing OCPP20 API.
        ret = m_events_handler.onDataTransfer(request, response, error_code, error_message);
    }

    return ret;
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
