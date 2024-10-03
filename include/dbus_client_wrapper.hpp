#pragma once

#include <iostream>
#include <functional>
#include <dbus/dbus.h>

//
// Interface
//

class IDBusClient {
protected:
    // Call method
    virtual void callMethod(
        const char* service_name,
        const char* object_path,
        const char* interface_name,
        const char* method_name,
        const std::function<bool(DBusMessage*)>& appendArgs,
        const std::function<void(DBusMessage*)>& parseFunc)
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
    void callMethod(
        const char* service_name,
        const char* object_path,
        const char* interface_name,
        const char* method_name,
        const std::function<bool(DBusMessage*)>& appendArgs,
        const std::function<void(DBusMessage*)>& parseFunc)
        override
    {
        // Connection
        if (! this->conn) {
            return;
        }

        // Initialize here to avoid cross creation (related to goto)
        DBusMessage* method_call = nullptr;
        DBusMessage* reply = nullptr;

        // Compose remote procedure call
        if ( nullptr ==
            (method_call = dbus_message_new_method_call(
                service_name,
                object_path,
                interface_name,
                method_name)
            ) ) {
            std::cerr << "ERROR: dbus_message_new_method_call - Unable to allocate memory for the message!" << std::endl;
            goto UNREF_METHOD;
        }

        // Append arguments
        if (appendArgs) {
            if ( false == appendArgs(method_call) ) {
                goto UNREF_METHOD;
            }
        }

        // Send method call
        if ( nullptr ==
            (reply = dbus_connection_send_with_reply_and_block(
                this->conn,
                method_call,
                DBUS_TIMEOUT_USE_DEFAULT,
                &(this->error))
            ) ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
            goto UNREF_REPLY;
        }

        // Parse response and Store
        if (parseFunc) {
            parseFunc(reply);
        }

        // Release
UNREF_REPLY:
        if (reply) {
            dbus_message_unref(reply);
        }
UNREF_METHOD:
        if (method_call) {
            dbus_message_unref(method_call);
        }
    }
};
