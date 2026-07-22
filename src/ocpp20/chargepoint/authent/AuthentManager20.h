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

#ifndef OPENOCPP_OCPP20_AUTHENTMANAGER20_H
#define OPENOCPP_OCPP20_AUTHENTMANAGER20_H

#include "IAuthentManager20.h"

#include <memory>

namespace ocpp
{
namespace config
{
class IInternalConfigManager;
} // namespace config
namespace database
{
class Database;
} // namespace database
namespace messages
{
class GenericMessagesConverter;
class GenericMessageSender;
class IMessageDispatcher;
} // namespace messages

namespace chargepoint
{
namespace ocpp20
{

class AuthentCache20;
class AuthentLocalList20;
class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 charge point authentication requests */
class AuthentManager20 : public IAuthentManager20
{
  public:
    /** @brief Constructor */
    AuthentManager20(ocpp::database::Database&                       database,
                     IChargePointEventsHandler20&                    events_handler,
                     ocpp::config::IInternalConfigManager&           internal_config,
                     const ocpp::messages::GenericMessagesConverter& messages_converter,
                     ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                     ocpp::messages::GenericMessageSender&           msg_sender);

    /** @brief Destructor */
    virtual ~AuthentManager20();

    // IAuthentManager20 interface

    /** @copydoc bool IAuthentManager20::authorize(const ocpp::types::ocpp20::IdTokenType&,
     *                                             ocpp::types::ocpp20::IdTokenInfoType&,
     *                                             std::string&,
     *                                             std::string&)
     */
    bool authorize(const ocpp::types::ocpp20::IdTokenType& id_token,
                   ocpp::types::ocpp20::IdTokenInfoType&  token_info,
                   std::string&                           error,
                   std::string&                           message) override;

    /** @copydoc void IAuthentManager20::update(const ocpp::types::ocpp20::IdTokenType&,
     *                                          const ocpp::types::ocpp20::IdTokenInfoType&)
     */
    void update(const ocpp::types::ocpp20::IdTokenType& id_token, const ocpp::types::ocpp20::IdTokenInfoType& token_info) override;

  private:
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Authorization cache */
    std::unique_ptr<AuthentCache20> m_cache;
    /** @brief Local authorization list */
    std::unique_ptr<AuthentLocalList20> m_local_list;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_AUTHENTMANAGER20_H
