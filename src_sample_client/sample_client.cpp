/*

Check D-Bus Policy Configuration: 

Ensure that the D-Bus configuration allows your user to call the method on the specified interface. The configuration files are usually located in /etc/dbus-1/system.d/ or /etc/dbus-1/session.d/. You might need to add or modify a policy rule to allow your method call. Example policy rule:

XML

<policy user="your-username">
    <allow send_destination="org.freedesktop.DBus" send_interface="org.freedesktop.DBus.Debug.Stats" />
</policy>

*/

#include <iostream>
#include <unordered_map>

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_client_wrapper.hpp"

class Introspectable : public DBusClient {
private:
    const char* service_name;// const no need to free
    const char* object_path;// const no need to free
    const char* interface_name;// const no need to free
    const char* response = nullptr;

public:
    // Constructor
    Introspectable(DBusConnection* dc):
        DBusClient(dc),
        service_name("org.freedesktop.DBus"),
        object_path("/"),
        interface_name("org.freedesktop.DBus.Introspectable")
        {}
    // delete
    Introspectable(Introspectable&& other) = delete;
    Introspectable& operator=(Introspectable&& other) = delete;
    Introspectable(const Introspectable&) = delete;
    Introspectable& operator=(const Introspectable&) = delete;
    // De-Constructor
    ~Introspectable() {}

public:
    void callIntrospectable() {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Introspect",
            nullptr,
            [this] (DBusMessage* reply) {
                this->Introspectable::parseIntrospectable(reply);
            }
        );
    }

private:
    // s
    void parseIntrospectable(DBusMessage* reply) {
        const char* response = nullptr;
        if ( false == dbus_message_get_args(
                reply,
                &(this->error),
                DBUS_TYPE_STRING,
                &response,
                DBUS_TYPE_INVALID)
            ) {
            std::cout << this->error.name << std::endl << this->error.message << std::endl;
        }
        std::cout << "Connected to D-Bus as \"" << ::dbus_bus_get_unique_name(this->conn) << "\"." << std::endl;
        std::cout << "Introspection Result:" << std::endl << std::endl;
        std::cout << response << std::endl;
    }
};

class DebugStats : public DBusClient {
private:
    const char* service_name;// const no need to free
    const char* object_path;// const no need to free
    const char* interface_name;// const no need to free

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
    void callGetStats() {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "GetStats",
            nullptr,
            [this] (DBusMessage* reply) {
                this->DebugStats::parseGetStats(reply);
            }
        );
    }

private:
    // a{sv}
    void parseGetStats(DBusMessage* reply) {
        std::unordered_map<const char*, unsigned int> arr;

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

        for (const auto& pr : arr) {
            std::cout << "Key: " << pr.first << ", Value: " << pr.second << std::endl;
        }
    }
};

int main() {
    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    // DBus.Introspectable
    //Introspectable intro(dbus_conn.getConn());
    //intro.callIntrospectable();

    // DBus.Debug.Stats 
    DebugStats debug_stats(dbus_conn.getConn());
    debug_stats.callGetStats();
    /*
    IDBusClient* ptr = &debug_stats;
    DebugStats* derived_ptr = dynamic_cast<DebugStats*>(ptr);
    if (derived_ptr) {
        derived_ptr->callmethodGetStats();
    }
    */

    return 0;
}