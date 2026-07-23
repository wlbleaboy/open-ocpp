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

#ifndef OPENOCPP_OCPP21_REQUESTFIFO21_H
#define OPENOCPP_OCPP21_REQUESTFIFO21_H

#include "Database.h"
#include "IRequestFifo.h"

#include <mutex>
#include <queue>

namespace ocpp
{
namespace chargepoint
{
namespace ocpp21
{

/** @brief Persistent FIFO for OCPP 2.1 offline requests */
class RequestFifo21 : public ocpp::messages::IRequestFifo
{
  public:
    RequestFifo21(ocpp::database::Database& database);
    virtual ~RequestFifo21();

    void initDatabaseTable();

    void push(unsigned int connector_id, const std::string& action, const rapidjson::Document& payload) override;
    bool front(unsigned int& connector_id, std::string& action, rapidjson::Document& payload) override;
    void pop() override;
    size_t size() const override;
    bool empty() const override { return (size() == 0); }
    void registerListener(IListener* listener) override { m_listener = listener; }

  private:
    static constexpr unsigned int MAX_ENTRIES_COUNT = 10000u;

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

    ocpp::database::Database& m_database;
    std::unique_ptr<ocpp::database::Database::Query> m_delete_query;
    std::unique_ptr<ocpp::database::Database::Query> m_insert_query;
    mutable std::mutex m_mutex;
    std::queue<Entry> m_fifo;
    unsigned int m_id;
    IListener* m_listener;

    void load();
};

} // namespace ocpp21
} // namespace chargepoint
} // namespace ocpp

#endif // OPENOCPP_OCPP21_REQUESTFIFO21_H
