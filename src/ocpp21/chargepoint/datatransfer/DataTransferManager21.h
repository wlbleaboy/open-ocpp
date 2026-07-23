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

#ifndef OPENOCPP_OCPP21_DATATRANSFERMANAGER21_H
#define OPENOCPP_OCPP21_DATATRANSFERMANAGER21_H

#include "DataTransfer21.h"
#include "GenericMessageHandler.h"

namespace ocpp
{
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages
namespace chargepoint
{
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 charge point DataTransfer requests */
class DataTransferManager21
    : public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::DataTransferReq,
                                                   ocpp::messages::ocpp21::DataTransferConf>
{
  public:
    DataTransferManager21(IChargePointEventsHandler21&                         events_handler,
                          const ocpp::messages::GenericMessagesConverter&      messages_converter,
                          ocpp::messages::IMessageDispatcher&                  msg_dispatcher,
                          ocpp::messages::GenericMessageSender&                msg_sender);
    virtual ~DataTransferManager21();

    bool call(const ocpp::messages::ocpp21::DataTransferReq& request,
              ocpp::messages::ocpp21::DataTransferConf&      response,
              std::string&                                   error,
              std::string&                                   message);

    bool handleMessage(const ocpp::messages::ocpp21::DataTransferReq& request,
                       ocpp::messages::ocpp21::DataTransferConf&      response,
                       std::string&                                   error_code,
                       std::string&                                   error_message) override;

  private:
    IChargePointEventsHandler21& m_events_handler;
    ocpp::messages::GenericMessageSender& m_msg_sender;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_DATATRANSFERMANAGER21_H
