#pragma once

#include <iostream>
#include <functional>
#include <dbus/dbus.h>

//
// Interface
//

class IRouter {
public:
   virtual DBusMessage* handleRequest(DBusMessage* message) = 0;
};

class IDBusServer {
public:
    virtual void run() = 0;

protected:
    virtual void runSession(DBusMessage* message) = 0;
};

//
// Abstract class
//

class DBusServer : public IDBusServer {
protected:
    DBusError error;
protected:
    DBusConnection* conn;
protected:
    bool runnable;
protected:
    const char* service_name;
protected:
    IRouter* controller;

public:
    // Constructor
    DBusServer(DBusConnection* dc, const char* sn, IRouter* ctl) :
        conn(dc),
        runnable(true),
        service_name(sn),
        controller(ctl)
    {
        dbus_error_init(&error);

        // Request a name on the bus
        if ( false == dbus_bus_request_name(this->conn, this->service_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &(this->error)) ) {
            this->runnable = false;
            std::cerr << "Name Error: " << this->error.message << std::endl;
        }
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

public:
    virtual void run() {
        // Check runnable
        if (false == this->runnable) {
            return;
        }

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

            DBusMessage* message = dbus_connection_pop_message(this->conn);
            if (nullptr == message) {
                continue;
            }

            dbus_connection_send(this->conn, dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found"), nullptr);

            dbus_message_unref(message);
        }
    }

protected:
    void runSession(DBusMessage* message) override {
        // Generate response
        DBusMessage* reply = controller->handleRequest(message);
        if (nullptr == reply) {
            return;
        }

        // Send reply
        dbus_connection_send(this->conn, reply, nullptr);
            
        // Release reply
        dbus_message_unref(reply);
    }
};
