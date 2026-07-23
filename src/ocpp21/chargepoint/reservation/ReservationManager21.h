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

#ifndef OPENOCPP_OCPP21_RESERVATIONMANAGER21_H
#define OPENOCPP_OCPP21_RESERVATIONMANAGER21_H

#include "CancelReservation21.h"
#include "Database.h"
#include "GenericMessageHandler.h"
#include "IReservationManager21.h"
#include "ReservationStatusUpdate21.h"
#include "ReservationUpdateStatusEnumType21.h"
#include "ReserveNow21.h"
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
namespace ocpp21
{

class IChargePointEventsHandler21;

/** @brief Handle OCPP 2.1 reservation requests */
class ReservationManager21
    : public IReservationManager21,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::ReserveNowReq, ocpp::messages::ocpp21::ReserveNowConf>,
      public ocpp::messages::GenericMessageHandler<ocpp::messages::ocpp21::CancelReservationReq,
                                                   ocpp::messages::ocpp21::CancelReservationConf>
{
  public:
    ReservationManager21(ocpp::database::Database&                       database,
                         IChargePointEventsHandler21&                    events_handler,
                         const ocpp::messages::GenericMessagesConverter& messages_converter,
                         ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                         ocpp::messages::GenericMessageSender&           msg_sender,
                         ocpp::helpers::ITimerPool&                      timer_pool,
                         ocpp::helpers::WorkerThreadPool&                worker_pool);
    virtual ~ReservationManager21();

    bool isTransactionAllowed(unsigned int evse_id, const ocpp::types::ocpp21::IdTokenType& id_token) override;
    void transactionStarted(unsigned int evse_id, const ocpp::types::ocpp21::IdTokenType& id_token) override;

    bool reservationStatusUpdate(const ocpp::messages::ocpp21::ReservationStatusUpdateReq& request,
                                 ocpp::messages::ocpp21::ReservationStatusUpdateConf&      response,
                                 std::string&                                              error,
                                 std::string&                                              message);

    bool handleMessage(const ocpp::messages::ocpp21::ReserveNowReq& request,
                       ocpp::messages::ocpp21::ReserveNowConf&      response,
                       std::string&                                 error_code,
                       std::string&                                 error_message) override;
    bool handleMessage(const ocpp::messages::ocpp21::CancelReservationReq& request,
                       ocpp::messages::ocpp21::CancelReservationConf&      response,
                       std::string&                                        error_code,
                       std::string&                                        error_message) override;

  private:
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

    ocpp::database::Database&                 m_database;
    IChargePointEventsHandler21&              m_events_handler;
    ocpp::messages::GenericMessageSender&     m_msg_sender;
    ocpp::helpers::WorkerThreadPool&          m_worker_pool;
    ocpp::helpers::Timer                      m_expiry_timer;
    std::mutex                                m_mutex;
    std::unordered_map<int, Reservation>      m_reservations;
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    std::unique_ptr<ocpp::database::Database::Query> m_update_query;
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;

    void initDatabaseTable();
    void loadReservations();
    void saveReservation(const Reservation& reservation);
    void removeReservation(int reservation_id);
    void checkExpiries();
    void endReservation(int reservation_id, ocpp::types::ocpp21::ReservationUpdateStatusEnumType status);
    void sendReservationStatusUpdate(int reservation_id, ocpp::types::ocpp21::ReservationUpdateStatusEnumType status);
    bool tokenMatches(const Reservation& reservation, const ocpp::types::ocpp21::IdTokenType& id_token) const;
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_RESERVATIONMANAGER21_H
