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

#include "ReservationManager21.h"

#include "GenericMessageSender.h"
#include "IChargePointEventsHandler21.h"
#include "IMessageDispatcher.h"
#include "Logger.h"
#include "ReservationStatusUpdate21.h"
#include "WorkerThreadPool.h"

#include <functional>

using namespace ocpp::database;
using namespace ocpp::messages;
using namespace ocpp::messages::ocpp21;
using namespace ocpp::types;
using namespace ocpp::types::ocpp21;

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

ReservationManager21::ReservationManager21(ocpp::database::Database&                       database,
                                           IChargePointEventsHandler21&                    events_handler,
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
      m_expiry_timer(timer_pool, "OCPP2.1 reservation expiry"),
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

    m_expiry_timer.setCallback([this] { m_worker_pool.run<void>(std::bind(&ReservationManager21::checkExpiries, this)); });
    m_expiry_timer.start(std::chrono::seconds(10));
}

ReservationManager21::~ReservationManager21()
{
    m_expiry_timer.stop();
}

bool ReservationManager21::isTransactionAllowed(unsigned int evse_id, const IdTokenType& id_token)
{
    bool ret = true;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& reservation_pair : m_reservations)
    {
        const Reservation& reservation = reservation_pair.second;
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

void ReservationManager21::transactionStarted(unsigned int evse_id, const IdTokenType& id_token)
{
    std::vector<int> reservations_to_remove;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& reservation_pair : m_reservations)
        {
            const int          reservation_id = reservation_pair.first;
            const Reservation& reservation    = reservation_pair.second;
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

bool ReservationManager21::reservationStatusUpdate(const ReservationStatusUpdateReq& request,
                                                   ReservationStatusUpdateConf&      response,
                                                   std::string&                      error,
                                                   std::string&                      message)
{
    return (m_msg_sender.call(RESERVATIONSTATUSUPDATE_ACTION, request, response, error, message) == CallResult::Ok);
}

bool ReservationManager21::handleMessage(const ReserveNowReq& request,
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
    reservation.id_token_type       = request.idToken.type.str();
    reservation.group_id_token      = "";
    reservation.group_id_token_type = "";
    reservation.expiry              = request.expiryDateTime;
    if (request.groupIdToken.isSet())
    {
        reservation.group_id_token      = request.groupIdToken.value().idToken.str();
        reservation.group_id_token_type = request.groupIdToken.value().type.str();
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
        for (const auto& reservation_pair : m_reservations)
        {
            const int          reservation_id = reservation_pair.first;
            const Reservation& existing       = reservation_pair.second;
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

    if (!reservation_accepted)
    {
        return true;
    }

    return true;
}

bool ReservationManager21::handleMessage(const CancelReservationReq& request,
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

void ReservationManager21::initDatabaseTable()
{
    auto query = m_database.query("CREATE TABLE IF NOT EXISTS Reservations21 ("
                                  "[reservation_id] INTEGER,"
                                  "[evse_id] INTEGER,"
                                  "[id_token] VARCHAR(255),"
                                  "[id_token_type] VARCHAR(20),"
                                  "[group_id_token] VARCHAR(255),"
                                  "[group_id_token_type] VARCHAR(20),"
                                  "[expiry] INTEGER,"
                                  "PRIMARY KEY([reservation_id]));");
    if (query && !query->exec())
    {
        LOG_ERROR << "Could not create OCPP2.1 reservations table : " << query->lastError();
    }

    m_insert_query = m_database.query("INSERT INTO Reservations21 VALUES (?, ?, ?, ?, ?, ?, ?);");
    m_update_query = m_database.query("UPDATE Reservations21 SET [evse_id]=?, [id_token]=?, [id_token_type]=?, "
                                      "[group_id_token]=?, [group_id_token_type]=?, [expiry]=? WHERE reservation_id=?;");
    m_delete_query = m_database.query("DELETE FROM Reservations21 WHERE reservation_id=?;");
}

void ReservationManager21::loadReservations()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_reservations.clear();

    auto query = m_database.query("SELECT * FROM Reservations21 WHERE TRUE;");
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

    LOG_INFO << "OCPP2.1 reservations : " << m_reservations.size() << " reservation(s) pending";
}

void ReservationManager21::saveReservation(const Reservation& reservation)
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
                LOG_ERROR << "Could not insert OCPP2.1 reservation [" << reservation.reservation_id << "]";
            }
            m_insert_query->reset();
        }
    }
    else if (m_update_query)
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
            LOG_ERROR << "Could not update OCPP2.1 reservation [" << reservation.reservation_id << "]";
        }
        m_update_query->reset();
    }
}

void ReservationManager21::removeReservation(int reservation_id)
{
    if (m_delete_query)
    {
        m_delete_query->bind(0, reservation_id);
        if (!m_delete_query->exec())
        {
            LOG_ERROR << "Could not delete OCPP2.1 reservation [" << reservation_id << "]";
        }
        m_delete_query->reset();
    }
}

void ReservationManager21::checkExpiries()
{
    std::vector<int> expired_reservations;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DateTime                    now = DateTime::now();
        for (const auto& reservation_pair : m_reservations)
        {
            const int          reservation_id = reservation_pair.first;
            const Reservation& reservation    = reservation_pair.second;
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

void ReservationManager21::endReservation(int reservation_id, ReservationUpdateStatusEnumType status)
{
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        reservation = m_reservations.find(reservation_id);
        if (reservation != m_reservations.end())
        {
            m_reservations.erase(reservation);
            removeReservation(reservation_id);
            found = true;
        }
    }

    if (found)
    {
        sendReservationStatusUpdate(reservation_id, status);
    }
}

void ReservationManager21::sendReservationStatusUpdate(int reservation_id, ReservationUpdateStatusEnumType status)
{
    ReservationStatusUpdateReq request;
    request.reservationId           = reservation_id;
    request.reservationUpdateStatus = status;

    ReservationStatusUpdateConf response;
    std::string                 error;
    std::string                 message;
    const CallResult result = m_msg_sender.call(RESERVATIONSTATUSUPDATE_ACTION, request, response, error, message);
    if (result != CallResult::Ok)
    {
        LOG_WARNING << "ReservationStatusUpdate failed for reservation [" << reservation_id << "] : " << error << " - " << message;
    }
}

bool ReservationManager21::tokenMatches(const Reservation& reservation, const IdTokenType& id_token) const
{
    const std::string token      = id_token.idToken.str();
    const std::string token_type = id_token.type.str();

    return ((reservation.id_token == token) && (reservation.id_token_type == token_type)) ||
           (!reservation.group_id_token.empty() && (reservation.group_id_token == token) &&
            (reservation.group_id_token_type == token_type));
}

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp
