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

#include "ReservationManager20.h"

#include "GenericMessageSender.h"
#include "IChargePointEventsHandler20.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "ReservationStatusUpdate20.h"
#include "WorkerThreadPool.h"

#include <functional>

using namespace ocpp::database;
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
ReservationManager20::ReservationManager20(ocpp::database::Database&                       database,
                                           IChargePointEventsHandler20&                    events_handler,
                                           const ocpp::messages::GenericMessagesConverter& messages_converter,
                                           ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                                           ocpp::messages::GenericMessageSender&           msg_sender,
                                           ocpp::helpers::ITimerPool&                      timer_pool,
                                           ocpp::helpers::WorkerThreadPool&                worker_pool)
    : GenericMessageHandler<ReserveNowReq, ReserveNowConf>(RESERVENOW_ACTION, messages_converter),
      GenericMessageHandler<CancelReservationReq, CancelReservationConf>(CANCELRESERVATION_ACTION, messages_converter),
      m_database(database),
      m_events_handler(events_handler),
      m_msg_sender(msg_sender),
      m_worker_pool(worker_pool),
      m_expiry_timer(timer_pool, "Reservation expiry"),
      m_mutex(),
      m_reservations(),
      m_insert_query(),
      m_update_query(),
      m_delete_query()
{
    initDatabaseTable();
    loadReservations();

    msg_dispatcher.registerHandler(RESERVENOW_ACTION,
                                   *dynamic_cast<GenericMessageHandler<ReserveNowReq, ReserveNowConf>*>(this));
    msg_dispatcher.registerHandler(CANCELRESERVATION_ACTION,
                                   *dynamic_cast<GenericMessageHandler<CancelReservationReq, CancelReservationConf>*>(this));

    m_expiry_timer.setCallback([this] { m_worker_pool.run<void>(std::bind(&ReservationManager20::checkExpiries, this)); });
    m_expiry_timer.start(std::chrono::seconds(10));
}

/** @brief Destructor */
ReservationManager20::~ReservationManager20()
{
    m_expiry_timer.stop();
}

/** @copydoc bool IReservationManager20::isTransactionAllowed(unsigned int, const IdTokenType&) */
bool ReservationManager20::isTransactionAllowed(unsigned int evse_id, const IdTokenType& id_token)
{
    bool ret = true;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [_, reservation] : m_reservations)
    {
        if ((reservation.evse_id == 0) || (reservation.evse_id == static_cast<int>(evse_id)))
        {
            if (!tokenMatches(reservation, id_token))
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}

/** @copydoc void IReservationManager20::transactionStarted(unsigned int, const IdTokenType&) */
void ReservationManager20::transactionStarted(unsigned int evse_id, const IdTokenType& id_token)
{
    std::vector<int> reservations_to_remove;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [reservation_id, reservation] : m_reservations)
        {
            if (((reservation.evse_id == 0) || (reservation.evse_id == static_cast<int>(evse_id))) && tokenMatches(reservation, id_token))
            {
                reservations_to_remove.push_back(reservation_id);
            }
        }
    }

    for (int reservation_id : reservations_to_remove)
    {
        endReservation(reservation_id, ReservationUpdateStatusEnumType::Removed);
    }
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const ReserveNowReq&, ReserveNowConf&, std::string&, std::string&) */
bool ReservationManager20::handleMessage(const ReserveNowReq& request,
                                         ReserveNowConf&      response,
                                         std::string&         error_code,
                                         std::string&         error_message)
{
    (void)error_code;
    (void)error_message;

    Reservation reservation;
    reservation.reservation_id      = request.id;
    reservation.evse_id             = request.evseId.isSet() ? request.evseId.value() : 0;
    reservation.id_token            = request.idToken.idToken.str();
    reservation.id_token_type       = IdTokenEnumTypeHelper.toString(request.idToken.type);
    reservation.group_id_token      = "";
    reservation.group_id_token_type = "";
    reservation.expiry              = request.expiryDateTime;
    if (request.groupIdToken.isSet())
    {
        reservation.group_id_token      = request.groupIdToken.value().idToken.str();
        reservation.group_id_token_type = IdTokenEnumTypeHelper.toString(request.groupIdToken.value().type);
    }

    if (reservation.expiry <= DateTime::now())
    {
        response.status = ReserveNowStatusEnumType::Rejected;
        return true;
    }

    bool reservation_accepted = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        bool evse_occupied = false;
        for (const auto& [reservation_id, existing] : m_reservations)
        {
            if (reservation_id == reservation.reservation_id)
            {
                continue;
            }

            if ((existing.evse_id == 0) || (reservation.evse_id == 0) || (existing.evse_id == reservation.evse_id))
            {
                evse_occupied = true;
                break;
            }
        }

        if (evse_occupied)
        {
            response.status = ReserveNowStatusEnumType::Occupied;
        }
        else
        {
            saveReservation(reservation);
            m_reservations[reservation.reservation_id] = reservation;
            response.status                            = ReserveNowStatusEnumType::Accepted;
            reservation_accepted                       = true;
        }
    }

    if (reservation_accepted)
    {
        m_events_handler.reservationStarted(reservation.reservation_id, reservation.evse_id, request.idToken);
    }

    return true;
}

/** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const CancelReservationReq&, CancelReservationConf&, std::string&, std::string&) */
bool ReservationManager20::handleMessage(const CancelReservationReq& request,
                                         CancelReservationConf&      response,
                                         std::string&                error_code,
                                         std::string&                error_message)
{
    (void)error_code;
    (void)error_message;

    bool found = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        found = (m_reservations.find(request.reservationId) != m_reservations.end());
    }

    if (found)
    {
        endReservation(request.reservationId, ReservationUpdateStatusEnumType::Removed);
        response.status = CancelReservationStatusEnumType::Accepted;
    }
    else
    {
        response.status = CancelReservationStatusEnumType::Rejected;
    }

    return true;
}

/** @brief Initialize database */
void ReservationManager20::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS Reservations20 ("
                                  "[reservation_id] INTEGER,"
                                  "[evse_id] INTEGER,"
                                  "[id_token] VARCHAR(36),"
                                  "[id_token_type] VARCHAR(32),"
                                  "[group_id_token] VARCHAR(36),"
                                  "[group_id_token_type] VARCHAR(32),"
                                  "[expiry] INTEGER,"
                                  "PRIMARY KEY([reservation_id]));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2 reservations table : " << query->lastError();
    }

    m_insert_query = m_database.query("INSERT INTO Reservations20 VALUES (?, ?, ?, ?, ?, ?, ?);");
    m_update_query = m_database.query("UPDATE Reservations20 SET [evse_id]=?, [id_token]=?, [id_token_type]=?, "
                                      "[group_id_token]=?, [group_id_token_type]=?, [expiry]=? WHERE reservation_id=?;");
    m_delete_query = m_database.query("DELETE FROM Reservations20 WHERE reservation_id=?;");
}

/** @brief Load reservations */
void ReservationManager20::loadReservations()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_reservations.clear();

    auto query = m_database.query("SELECT * FROM Reservations20 WHERE TRUE;");
    if (query && query->exec() && query->hasRows())
    {
        do
        {
            Reservation reservation;
            reservation.reservation_id      = query->getInt32(0);
            reservation.evse_id             = query->getInt32(1);
            reservation.id_token            = query->getString(2);
            reservation.id_token_type       = query->getString(3);
            reservation.group_id_token      = query->getString(4);
            reservation.group_id_token_type = query->getString(5);
            reservation.expiry              = DateTime(query->getInt64(6));

            if (reservation.expiry <= DateTime::now())
            {
                removeReservation(reservation.reservation_id);
            }
            else
            {
                m_reservations[reservation.reservation_id] = reservation;
            }
        } while (query->next());
    }

    LOG_INFO << "OCPP2 reservations : " << m_reservations.size() << " reservation(s) pending";
}

/** @brief Save reservation */
void ReservationManager20::saveReservation(const Reservation& reservation)
{
    auto existing = m_reservations.find(reservation.reservation_id);
    if (existing == m_reservations.end())
    {
        if (m_insert_query)
        {
            m_insert_query->bind(0, reservation.reservation_id);
            m_insert_query->bind(1, reservation.evse_id);
            m_insert_query->bind(2, reservation.id_token);
            m_insert_query->bind(3, reservation.id_token_type);
            m_insert_query->bind(4, reservation.group_id_token);
            m_insert_query->bind(5, reservation.group_id_token_type);
            m_insert_query->bind(6, static_cast<int64_t>(reservation.expiry.timestamp()));
            if (!m_insert_query->exec())
            {
                LOG_ERROR << "Could not insert OCPP2 reservation [" << reservation.reservation_id << "]";
            }
            m_insert_query->reset();
        }
    }
    else
    {
        if (m_update_query)
        {
            m_update_query->bind(0, reservation.evse_id);
            m_update_query->bind(1, reservation.id_token);
            m_update_query->bind(2, reservation.id_token_type);
            m_update_query->bind(3, reservation.group_id_token);
            m_update_query->bind(4, reservation.group_id_token_type);
            m_update_query->bind(5, static_cast<int64_t>(reservation.expiry.timestamp()));
            m_update_query->bind(6, reservation.reservation_id);
            if (!m_update_query->exec())
            {
                LOG_ERROR << "Could not update OCPP2 reservation [" << reservation.reservation_id << "]";
            }
            m_update_query->reset();
        }
    }
}

/** @brief Remove reservation */
void ReservationManager20::removeReservation(int reservation_id)
{
    if (m_delete_query)
    {
        m_delete_query->bind(0, reservation_id);
        if (!m_delete_query->exec())
        {
            LOG_ERROR << "Could not delete OCPP2 reservation [" << reservation_id << "]";
        }
        m_delete_query->reset();
    }
}

/** @brief Check expired reservations */
void ReservationManager20::checkExpiries()
{
    std::vector<int> expired_reservations;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DateTime                    now = DateTime::now();
        for (const auto& [reservation_id, reservation] : m_reservations)
        {
            if (reservation.expiry <= now)
            {
                expired_reservations.push_back(reservation_id);
            }
        }
    }

    for (int reservation_id : expired_reservations)
    {
        endReservation(reservation_id, ReservationUpdateStatusEnumType::Expired);
    }
}

/** @brief End a reservation */
void ReservationManager20::endReservation(int reservation_id, ReservationUpdateStatusEnumType status)
{
    bool found = false;
    int  evse_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        reservation = m_reservations.find(reservation_id);
        if (reservation != m_reservations.end())
        {
            evse_id = reservation->second.evse_id;
            m_reservations.erase(reservation);
            removeReservation(reservation_id);
            found = true;
        }
    }

    if (found)
    {
        sendReservationStatusUpdate(reservation_id, status);
        m_events_handler.reservationEnded(reservation_id, evse_id, status);
    }
}

/** @brief Send ReservationStatusUpdate */
void ReservationManager20::sendReservationStatusUpdate(int reservation_id, ReservationUpdateStatusEnumType status)
{
    ReservationStatusUpdateReq request;
    request.reservationId           = reservation_id;
    request.reservationUpdateStatus = status;

    ReservationStatusUpdateConf response;
    std::string                 error;
    std::string                 message;
    CallResult result = m_msg_sender.call(RESERVATIONSTATUSUPDATE_ACTION, request, response, error, message);
    if (result != CallResult::Ok)
    {
        LOG_WARNING << "ReservationStatusUpdate failed for reservation [" << reservation_id << "] : " << error << " - " << message;
    }
}

/** @brief Check token match */
bool ReservationManager20::tokenMatches(const Reservation& reservation, const IdTokenType& id_token) const
{
    const std::string token      = id_token.idToken.str();
    const std::string token_type = IdTokenEnumTypeHelper.toString(id_token.type);

    return ((reservation.id_token == token) && (reservation.id_token_type == token_type)) ||
           (!reservation.group_id_token.empty() && (reservation.group_id_token == token) &&
            (reservation.group_id_token_type == token_type));
}

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp
