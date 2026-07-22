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

#ifndef OPENOCPP_OCPP20_REQUESTFIFO20_H
#define OPENOCPP_OCPP20_REQUESTFIFO20_H

#include "Database.h"
#include "IRequestFifo.h"

#include <mutex>
#include <queue>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp20
{

/** @brief Persistent FIFO for OCPP 2.0.1 offline requests */
class RequestFifo20 : public ocpp::messages::IRequestFifo
{
  public:
    /** @brief Constructor */
    RequestFifo20(ocpp::database::Database& database);

    /** @brief Destructor */
    virtual ~RequestFifo20();

    /** @brief Initialize the database table */
    void initDatabaseTable();

    // IRequestFifo interface

    /** @copydoc void IRequestFifo::push(unsigned int, const std::string&, const rapidjson::Document&) */
    void push(unsigned int connector_id, const std::string& action, const rapidjson::Document& payload) override;

    /** @copydoc bool IRequestFifo::front(unsigned int&, std::string&, rapidjson::Document&) */
    bool front(unsigned int& connector_id, std::string& action, rapidjson::Document& payload) override;

    /** @copydoc void IRequestFifo::pop() */
    void pop() override;

    /** @copydoc size_t IRequestFifo::size() const */
    size_t size() const override;

    /** @copydoc bool IRequestFifo::empty() const */
    bool empty() const override { return (size() == 0); }

    /** @copydoc void IRequestFifo::registerListener(IListener*) */
    void registerListener(IListener* listener) override { m_listener = listener; }

  private:
    /** @brief Maximum number of offline requests */
    static constexpr unsigned int MAX_ENTRIES_COUNT = 10000u;

    /** @brief FIFO entry */
    struct Entry
    {
        Entry() : id(0), connector_id(0), action(), request() { }
        Entry(unsigned int _id, unsigned int _connector_id, std::string _action, std::string _request)
            : id(_id), connector_id(_connector_id), action(_action), request(_request)
        {
        }

        unsigned int id;
        unsigned int connector_id;
        std::string action;
        std::string request;
    };

    /** @brief Database */
    ocpp::database::Database& m_database;
    /** @brief Delete query */
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    /** @brief Insert query */
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    /** @brief Mutex */
    mutable std::mutex m_mutex;
    /** @brief FIFO */
    std::queue<Entry> m_fifo;
    /** @brief Next id */
    unsigned int m_id;
    /** @brief Listener */
    IListener* m_listener;

    /** @brief Load stored requests */
    void load();
};

} // namespace ocpp20
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP20_REQUESTFIFO20_H
