/*
g++ sample_client.cpp -o sample_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

sudo service dbus --full-restart
sudo ./sample_client
*/

#include <iostream>
#include <unordered_map>
#include <dbus/dbus.h>

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
};

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
};

class DebugStats : public DBusClient {
private:
    const char* service_name;
    const char* object_path;
    const char* interface_name;
    std::unordered_map<const char*, unsigned int> arr;

public:
    // Constructor
    DebugStats(DBusConnection* dc):
        DBusClient(dc),
        service_name("org.freedesktop.DBus"),
        object_path("/org/freedesktop/DBus"),
        interface_name("org.freedesktop.DBus.Debug.Stats")
        {}
    // delete
    DebugStats(DebugStats&& other) = delete;
    DebugStats& operator=(DebugStats&& other) = delete;
    DebugStats(const DebugStats&) = delete;
    DBusClient& operator=(const DebugStats&) = delete;
    // De-Constructor
    ~DebugStats() {}

public:
    void callmethodGetStats() {
        // Connection
        if (! conn) {
            return;
        }

        // Compose remote procedure call
        DBusMessage* method_call = composeMethodCall(
            this->service_name,
            this->object_path,
            this->interface_name,
            "GetStats"
        );
        if (! method_call) {
            return;
        }

        // Send method call
        DBusMessage* reply = sendMethodCall(method_call);
        // Parse response and Store
        parseGetStats(reply);
        // Release reply
        dbus_message_unref(reply);

        // Execute callback
    }

public:
    void showGetStats() {
        for (const auto& pr : this->arr) {
            std::cout << "Key: " << pr.first << ", Value: " << pr.second << std::endl;
        }
    }

private:
    // a{sv}
    void parseGetStats(DBusMessage* reply) {
        DBusMessageIter iter;
        dbus_message_iter_init(reply, &iter);
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&iter)) {
            return;
        }

        DBusMessageIter subIter;
        for (dbus_message_iter_recurse(&iter, &subIter);
             dbus_message_iter_get_arg_type(&subIter) != DBUS_TYPE_INVALID;
             dbus_message_iter_next(&subIter))
        {
            DBusMessageIter dictEntry;
            dbus_message_iter_recurse(&subIter, &dictEntry);

            char* key;
            dbus_message_iter_get_basic(&dictEntry, &key);
            dbus_message_iter_next(&dictEntry);
            
            unsigned int value;
            DBusMessageIter variant;
            dbus_message_iter_recurse(&dictEntry, &variant);
            // check type
            //int valueType = dbus_message_iter_get_arg_type(&variant);
            //valueType == DBUS_TYPE_UINT32 or valueType == DBUS_TYPE_STRING
            dbus_message_iter_get_basic(&variant, &value);

            arr[key] = value;
        }
    }
};

int main() {
    DBusError error;
    dbus_error_init(&error);

    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    // DBus.Debug.Stats 
    DebugStats debug_stats(dbus_conn.getConn());
    debug_stats.callmethodGetStats();
    debug_stats.showGetStats();
    /*
    IDBusClient* ptr = &debug_stats;
    DebugStats* derived_ptr = dynamic_cast<DebugStats*>(ptr);
    if (derived_ptr) {
        derived_ptr->callmethodGetStats();
        derived_ptr->showGetStats();
    }
    */

    return 0;
}