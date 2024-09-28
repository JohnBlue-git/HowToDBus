#pragma once

#include <iostream>
#include <functional>
#include <dbus/dbus.h>

//
// Interface
//

class IDBusServer {
protected:
    // Run server
    virtual void run(
        const std::function<void()>& handleBlock)
        = 0;

    // Handle request
    virtual void handleRequest(
        const std::function<DBusMessage*(DBusMessage*)>& generateResponse)
        = 0;
};

//
// Abstract class
//

class DBusServer : public IDBusServer {
protected:
    DBusError error;
protected:
    DBusConnection* conn;

public:
    // Constructor
    DBusServer(DBusConnection* dc): conn(dc)
    {
        dbus_error_init(&error);
    }
    // delete
    DBusServer(DBusServer&& other) = delete;
    DBusServer& operator=(DBusServer&& other) = delete;
    DBusServer(const DBusServer&) = delete;
    DBusServer& operator=(const DBusServer&) = delete;
    // De-Constructor
    ~DBusServer() {
        dbus_error_free(&error);
    }

protected:
    void run(
        const std::function<void()>& handleBlock)
        override
    {
        // Main loop
        for (;;) {
            // Without blocking (drawback is busy waiting)
            //dbus_connection_read_write(this->conn, 1000);
            // will return true/false; can set timeout (ms)
            // should accompany with
            //#include <unistd.h>
            //usleep(1000);

            // Blocking way
            // useful to look at
            // spec: https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga580d8766c23fe5f49418bc7d87b67dc6
            dbus_connection_read_write_dispatch(this->conn, -1);

            // Handle request block
            handleBlock();
        }
    }

    void handleRequest(
        const std::function<DBusMessage*(DBusMessage*)>& generateResponse)
        override
    {
            // Initialize variables here to avoid crosses initialization (related to goto)
            DBusMessage* message = nullptr;
            DBusMessage* reply = nullptr;

            // Get message
            if ( nullptr == (message = dbus_connection_pop_message(this->conn)) ) {
                std::cout << this->error.name << std::endl << this->error.message << std::endl;
                goto UNREF_MESSAGE;
            }

            // Generate response
            reply = generateResponse(message);

            // Send reply
            if (reply) {
                dbus_connection_send(this->conn, reply, nullptr);
                // Release reply
                dbus_message_unref(reply);
            }

UNREF_MESSAGE:
            // Release message
            dbus_message_unref(message);
    }
};
