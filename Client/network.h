#pragma once

#include "queuedConnectionManager.h"
#include "queuedconnectionreader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "datagram.h"

#include "player.h"

class NetworkClient {
public:
    NetworkClient(const std::string& host, int port)
    {
        conn_mgr = new QueuedConnectionManager();
        conn_writer = new ConnectionWriter(conn_mgr, 0);
        conn_reader = new QueuedConnectionReader(conn_mgr, 0);

        NetAddress addr;
        if (!addr.set_host(host, port)) {
            std::cerr << "Invalid server address.\n";
            return;
        }

        connection = conn_mgr->open_TCP_client_connection(addr, 1.0);
        if (connection.is_null()) {
            std::cerr << "Failed to connect to server.\n";
            return;
        }

        conn_reader->add_connection(connection);
        AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("ListenToServer", &NetworkClient::listen_task, this));
    }


    ~NetworkClient() {
        if (!connection.is_null()) { conn_mgr->close_connection(connection); }
        delete conn_reader;
        delete conn_writer;
        delete conn_mgr;
    }

    void disconnect() {
        if (!connection.is_null()) { conn_mgr->close_connection(connection); }
        delete conn_reader;
        delete conn_writer;
        delete conn_mgr;
    }

    void onConnection(Datagram& dg) {
        if (connection.is_null()) {
            std::cerr << "No server connection to send data.\n";
            return;
        }

        conn_writer->send(dg, connection, false);
        std::cout << "onConnect called";
    }

private:
    QueuedConnectionManager* conn_mgr = nullptr;
    ConnectionWriter* conn_writer = nullptr;
    QueuedConnectionReader* conn_reader = nullptr;
    PT(Connection) connection;

    std::vector<std::unique_ptr<Player>> players;

private:
    static AsyncTask::DoneStatus listen_task(GenericAsyncTask* task, void* data) {
        return static_cast<NetworkClient*>(data)->handle_listen();
    }

    AsyncTask::DoneStatus handle_listen() {
        while (conn_reader->data_available()) {
            NetDatagram dg;
            if (!conn_reader->get_data(dg)) continue;

            const std::string data = dg.get_message();
            std::istringstream stream(data, std::ios::binary);

            uint8_t msg_type;
            stream.read(reinterpret_cast<char*>(&msg_type), sizeof(msg_type));

            if (msg_type == 1) { // NewPlayerConnected
                uint8_t name_len;
                stream.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));

                std::string name(name_len, '\0');
                stream.read(&name[0], name_len);

                int car_id, color_id;
                stream.read(reinterpret_cast<char*>(&car_id), sizeof(car_id));
                stream.read(reinterpret_cast<char*>(&color_id), sizeof(color_id));

                NodePath dummy_mesh("dummy");
                auto player = std::make_unique<Player>(name, car_id, static_cast<p_color>(color_id), dummy_mesh);
                players.push_back(std::move(player));

                std::cout << "Player connected: " << name << " (Car " << car_id << ", Color " << color_id << ")\n";
            }
        }
        return AsyncTask::DS_cont;
    }
};
