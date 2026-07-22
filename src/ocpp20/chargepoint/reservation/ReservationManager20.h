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

#ifndef OPENOCPP_OCPP20_RESERVATIONMANAGER20_H
#define OPENOCPP_OCPP20_RESERVATIONMANAGER20_H

#include "CancelReservation20.h"
#include "Database.h"
#include "GenericMessageHandler.h"
#include "IReservationManager20.h"
#include "ReserveNow20.h"
#include "ReservationUpdateStatusEnumType20.h"
#include "Timer.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace ocpp
{
namespace messages
{
class GenericMessageSender;
class GenericMessagesConverter;
class IMessageDispatcher;
} // namespace messages
namespace helpers
{
class WorkerThreadPool;
} // namespace helpers

namespace chargepoint
{
namespace ocpp20
{

class IChargePointEventsHandler20;

/** @brief Handle OCPP 2.0.1 reservation requests */
class ReservationManager20
    : public IReservationManager20,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::ReserveNowReq, ocpp::messages::ocpp20::ReserveNowConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp20::CancelReservationReq,
                                                   ocpp::messages::ocpp20::CancelReservationConf>
{
  public:
    /** @brief Constructor */
    ReservationManager20(ocpp::database::Database&                       database,
                         IChargePointEventsHandler20&                    events_handler,
                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                         ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                         ocpp::messages::GenericMessageSender&           msg_sender,
                         ocpp::helpers::ITimerPool&                      timer_pool,
                         ocpp::helpers::WorkerThreadPool&                worker_pool);

    /** @brief Destructor */
    virtual ~ReservationManager20();

    // IReservationManager20 interface

    bool isTransactionAllowed(unsigned int evse_id, const ocpp::types::ocpp20::IdTokenType& id_token) override;
    void transactionStarted(unsigned int evse_id, const ocpp::types::ocpp20::IdTokenType& id_token) override;

    // GenericMessageHandler interface

    bool handleMessage(const ocpp::messages::ocpp20::ReserveNowReq& request,
                       ocpp::messages::ocpp20::ReserveNowConf&      response,
                       std::string&                                 error_code,
                       std::string&                                 error_message) override;

    bool handleMessage(const ocpp::messages::ocpp20::CancelReservationReq& request,
                       ocpp::messages::ocpp20::CancelReservationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

  private:
    /** @brief Reservation data */
    struct Reservation
    {
        int reservation_id;
        int evse_id;
        std::string id_token;
        std::string id_token_type;
        std::string group_id_token;
        std::string group_id_token_type;
        ocpp::types::DateTime expiry;
    };

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Events handler */
    IChargePointEventsHandler20& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
    /** @brief Worker pool */
    ocpp::helpers::WorkerThreadPool& m_worker_pool;
    /** @brief Expiry timer */
    ocpp::helpers::Timer m_expiry_timer;
    /** @brief Mutex */
    std::mutex m_mutex;
    /** @brief Reservations indexed by id */
    std::unordered_map<int, Reservation> m_reservations;
    /** @brief Insert query */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    /** @brief Update query */
    std::unique_ptr<ocpp::database::Database::Query> m_update_query;
    /** @brief Delete query */
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;

    /** @brief Initialize database */
    void initDatabaseTable();
    /** @brief Load reservations */
    void loadReservations();
    /** @brief Save reservation */
    void saveReservation(const Reservation& reservation);
    /** @brief Remove reservation */
    void removeReservation(int reservation_id);
    /** @brief Check expired reservations */
    void checkExpiries();
    /** @brief End a reservation */
    void endReservation(int reservation_id, ocpp::types::ocpp20::ReservationUpdateStatusEnumType status);
    /** @brief Send ReservationStatusUpdate */
    void sendReservationStatusUpdate(int reservation_id, ocpp::types::ocpp20::ReservationUpdateStatusEnumType status);
    /** @brief Check token match */
    bool tokenMatches(const Reservation& reservation, const ocpp::types::ocpp20::IdTokenType& id_token) const;
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_RESERVATIONMANAGER20_H
