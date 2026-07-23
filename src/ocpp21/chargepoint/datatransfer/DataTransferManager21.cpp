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

#include "DataTransferManager21.h"
#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"

using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

DataTransferManager21::DataTransferManager21(IChargePointEventsHandler21&                    events_handler,
                                             const ocpp::messages::GenericMessagesConverter& messages_converter,
                                             ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                             ocpp::messages::GenericMessageSender&           msg_sender)
    : GenericMessageHandler<DataTransferReq, DataTransferConf>(DATATRANSFER_ACTION, messages_converter),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender)
{
    msg_dispatcher.registerHandler(DATATRANSFER_ACTION, *dynamic_cast<GenericMessageHandler<DataTransferReq, DataTransferConf>*>(this));
}

DataTransferManager21::~DataTransferManager21()
{
}

bool DataTransferManager21::call(const DataTransferReq& request, DataTransferConf& response, std::string& error, std::string& message)
{
    return (m_msg_sender.call(DATATRANSFER_ACTION, request, response, error, message) == CallResult::Ok);
}

bool DataTransferManager21::handleMessage(const DataTransferReq& request,
                                          DataTransferConf&      response,
                                          std::string&           error_code,
                                          std::string&           error_message)
{
    return m_events_handler.onDataTransfer(request, response, error_code, error_message);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
