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

#ifndef OPENOCPP_OCPP20_DATATRANSFERMANAGER20_H
#define OPENOCPP_OCPP20_DATATRANSFERMANAGER20_H

#include "DataTransfer20.h"
#include "GenericMessageHandler.h"
#include "IDataTransferManager20.h"

#include <unordered_map>

namespace ocpp
{
namespace messages
{
class IMessageDispatcher;
class GenericMessagesConverter;
class GenericMessageSender;
} // namespace messages

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 charge point DataTransfer requests */
class DataTransferManager20
    : public IDataTransferManager20,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::DataTransferReq,
                                                   ocpp::messages::ocpp20::DataTransferConf>
{
  public:
    /** @brief Constructor */
    DataTransferManager20(IChargePointEventsHandler20&                         events_handler,
                          const ocpp::messages::GenericMessagesConverter&      messages_converter,
                          ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
                          ocpp::messages::GenericMessageSender&                msg_sender);

    /** @brief Destructor */
    virtual ~DataTransferManager20();

    /**
     * @brief Send a data transfer request
     * @param vendor_id Identifies the vendor specific implementation
     * @param message_id Identifies the message
     * @param request_data Data associated to the request
     * @param status Response status
     * @param response_data Data associated with the response
     * @param error Error (Empty if not a CallError)
     * @param message Error message (Empty if not a CallError)
     * @return true if the data transfer has been done, false otherwise
     */
    bool dataTransfer(const std::string&                                  vendor_id,
                      const std::string&                                  message_id,
                      const std::string&                                  request_data,
                      ocpp::types::ocpp20::DataTransferStatusEnumType&    status,
                      std::string&                                        response_data,
                      std::string&                                        error,
                      std::string&                                        message);

    // IDataTransferManager20 interface

    /** @copydoc void IDataTransferManager20::registerHandler(const std::string&, IDataTransferHandler&) */
    void registerHandler(const std::string& vendor_id, IDataTransferHandler& handler) override;

    // GenericMessageHandler interface

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                std::string& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::ocpp20::DataTransferReq& request,
                       ocpp::messages::ocpp20::DataTransferConf&      response,
                       std::string&                                   error_code,
                       std::string&                                   error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Registered handlers */
    std::unordered_map<std::string, IDataTransferHandler*> m_handlers;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_DATATRANSFERMANAGER20_H
