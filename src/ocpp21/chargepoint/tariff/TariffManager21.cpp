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

#include "TariffManager21.h"

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

TariffManager21::TariffManager21(IChargePointEventsHandler21&                    events_handler,
                                 const ocpp::messages::GenericMessagesConverter& messages_converter,
                                 ocpp::messages::IMessageDispatcher&             msg_dispatcher)
    : GenericMessageHandler<ClearTariffsReq, ClearTariffsConf>(CLEARTARIFFS_ACTION, messages_converter),
      GenericMessageHandler<GetTariffsReq, GetTariffsConf>(GETTARIFFS_ACTION, messages_converter),
      m_events_handler(events_handler)
{
    msg_dispatcher.registerHandler(CLEARTARIFFS_ACTION,
                                   *dynamic_cast<GenericMessageHandler<ClearTariffsReq, ClearTariffsConf>*>(this));
    msg_dispatcher.registerHandler(GETTARIFFS_ACTION,
                                   *dynamic_cast<GenericMessageHandler<GetTariffsReq, GetTariffsConf>*>(this));
}

TariffManager21::~TariffManager21() { }

bool TariffManager21::handleMessage(const ClearTariffsReq& request,
                                    ClearTariffsConf&      response,
                                    std::string&           error_code,
                                    std::string&           error_message)
{
    return m_events_handler.onClearTariffs(request, response, error_code, error_message);
}

bool TariffManager21::handleMessage(const GetTariffsReq& request,
                                    GetTariffsConf&      response,
                                    std::string&         error_code,
                                    std::string&         error_message)
{
    return m_events_handler.onGetTariffs(request, response, error_code, error_message);
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
