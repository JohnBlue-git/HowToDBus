#pragma once

#include <iostream>
#include <unordered_map>
#include <dbus/dbus.h>

//
// DBusConn
//

class DBusConn {
private:
    DBusError error;
private:
    DBusConnection* conn;

public:
    // Constructor
    DBusConn(DBusBusType type)
    {
        dbus_error_init(&error);

        // Connect to D-Bus
        if ( nullptr == (conn = dbus_bus_get(type, &error)) ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
        }
    }
    // delete
    DBusConn(DBusConn&& other) = delete;
    DBusConn& operator=(DBusConn&& other) = delete;
    DBusConn(const DBusConn&) = delete;
    DBusConn& operator=(const DBusConn&) = delete;
    // De-Constructor
    ~DBusConn() {
        dbus_error_free(&error);

        // Applications must not close shared connections -
        // see dbus_connection_close() docs. This is a bug in the application.
        // dbus_connection_close(dbus_conn);

        // When using the System Bus, unreference the connection instead of closing it
        if (conn) {
            dbus_connection_unref(conn);
        }
    }

public:
    DBusConnection* getConn() {
        return conn;
    };
};

//
// Interface
//

class IDBusClient {
protected:
    // Compose remote procedure call
    virtual DBusMessage* composeMethodCall(
        const char* service_name,
        const char* object_path,
        const char* interface_name,
        const char* method_name)
        = 0;

    // Append argument on remote procedure call
    virtual void appendArg(
        DBusMessage* method_call,
        int arg_type,
        void* arg)
        = 0;

    // Invoke remote procedure call, block for response
    virtual DBusMessage* sendMethodCall(
        DBusMessage* method_call)
        = 0;

    // Send reply
    virtual void sendReply(
        DBusMessage* reply)
        = 0;
};

//
// Abstract class
//

class DBusClient : public IDBusClient {
protected:
    DBusError error;
protected:
    DBusConnection* conn;

public:
    // Constructor
    DBusClient(DBusConnection* dc): conn(dc)
    {
        dbus_error_init(&error);
    }
    // delete
    DBusClient(DBusClient&& other) = delete;
    DBusClient& operator=(DBusClient&& other) = delete;
    DBusClient(const DBusClient&) = delete;
    DBusClient& operator=(const DBusClient&) = delete;
    // De-Constructor
    ~DBusClient() {
        dbus_error_free(&error);
    }

protected:
    DBusMessage* composeMethodCall(
        const char* service_name,
        const char* object_path,
        const char* interface_name,
        const char* method_name)
        override
    {
        DBusMessage* method_call = dbus_message_new_method_call(
            service_name,
            object_path,
            interface_name,
            method_name
        );
        if (! method_call) {
            std::cerr << "ERROR: dbus_message_new_method_call - Unable to allocate memory for the message!" << std::endl;
            return nullptr;
        }
        return method_call;
    }

    void appendArg(
        DBusMessage* method_call,
        int arg_type,
        void* arg)
        override
    {
        bool check = dbus_message_append_args(
            method_call,
            arg_type,
            arg,
            DBUS_TYPE_INVALID
        );
        if (! check) {
            std::cerr << "ERROR: dbus_message_append_args - Unable to append arguments" << std::endl;
        }
    }

    DBusMessage* sendMethodCall(
        DBusMessage* method_call)
        override
    {
        DBusMessage* reply = dbus_connection_send_with_reply_and_block(
            this->conn,
            method_call,
            DBUS_TIMEOUT_USE_DEFAULT,
            &(this->error)
        );
        if (! reply) {
            std::cerr << error.name << std::endl << error.message << std::endl;
        }
        return reply;
    }

    void sendReply(
        DBusMessage* reply)
        override
    {
        if (! reply) {
            return;
        }
        //       DBusConnection*, DBusMessage*, *client_serial: A pointer to a variable to store the message serial number, or NULL if you don’t need the serial number.
        dbus_connection_send(this->conn, reply, nullptr);
    }
};
