#include <iostream>
#include <iomanip>
#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_client_wrapper.hpp"

//
// Property Client
//

class PropertyClient : public DBusClient {
private:
    const char* service_name;
    const char* object_path;
    const char* interface_name;
    const char* properties_interface;

public:
    // Constructor
    PropertyClient(DBusConnection* dc) :
        DBusClient(dc),
        service_name("com.example.PropertyService"),
        object_path("/com/example/PropertyService"),
        interface_name("com.example.PropertyInterface"),
        properties_interface("org.freedesktop.DBus.Properties")
    {}
    // delete
    PropertyClient() = delete;
    PropertyClient(PropertyClient&& other) = delete;
    PropertyClient& operator=(PropertyClient&& other) = delete;
    PropertyClient(const PropertyClient&) = delete;
    PropertyClient& operator=(const PropertyClient&) = delete;
    // De-Constructor
    ~PropertyClient() {}

public:
    void getIntProperty(const char* property_name) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->properties_interface,
            "Get",
            [this, &property_name](DBusMessage* method_call) {
                const char* interface = this->interface_name;
                if (!dbus_message_append_args(
                        method_call,
                        DBUS_TYPE_STRING, &interface,
                        DBUS_TYPE_STRING, &property_name,
                        DBUS_TYPE_INVALID)) {
                    std::cerr << "ERROR: Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [&property_name](DBusMessage* reply) {
                DBusMessageIter iter, variant_iter;
                int32_t value;

                if (!dbus_message_iter_init(reply, &iter)) {
                    std::cerr << "ERROR: Reply has no arguments!" << std::endl;
                    return;
                }

                dbus_message_iter_recurse(&iter, &variant_iter);
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_INT32) {
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    std::cout << "[GET] " << property_name << " = " << value << std::endl;
                } else {
                    std::cerr << "ERROR: Unexpected property type!" << std::endl;
                }
            });
    }

    void getStringProperty(const char* property_name) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->properties_interface,
            "Get",
            [this, &property_name](DBusMessage* method_call) {
                const char* interface = this->interface_name;
                if (!dbus_message_append_args(
                        method_call,
                        DBUS_TYPE_STRING, &interface,
                        DBUS_TYPE_STRING, &property_name,
                        DBUS_TYPE_INVALID)) {
                    std::cerr << "ERROR: Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [&property_name](DBusMessage* reply) {
                DBusMessageIter iter, variant_iter;
                const char* value;

                if (!dbus_message_iter_init(reply, &iter)) {
                    std::cerr << "ERROR: Reply has no arguments!" << std::endl;
                    return;
                }

                dbus_message_iter_recurse(&iter, &variant_iter);
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    std::cout << "[GET] " << property_name << " = " << value << std::endl;
                } else {
                    std::cerr << "ERROR: Unexpected property type!" << std::endl;
                }
            });
    }

    void setIntProperty(const char* property_name, int32_t value) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->properties_interface,
            "Set",
            [this, &property_name, &value](DBusMessage* method_call) {
                const char* interface = this->interface_name;
                DBusMessageIter iter, variant_iter;

                dbus_message_iter_init_append(method_call, &iter);
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property_name);
                
                dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "i", &variant_iter);
                dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_INT32, &value);
                dbus_message_iter_close_container(&iter, &variant_iter);

                return true;
            },
            [&property_name, &value](DBusMessage* reply) {
                std::cout << "[SET] " << property_name << " = " << value << " (success)" << std::endl;
            });
    }

    void setStringProperty(const char* property_name, const char* value) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->properties_interface,
            "Set",
            [this, &property_name, &value](DBusMessage* method_call) {
                const char* interface = this->interface_name;
                DBusMessageIter iter, variant_iter;

                dbus_message_iter_init_append(method_call, &iter);
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
                dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property_name);
                
                dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
                dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &value);
                dbus_message_iter_close_container(&iter, &variant_iter);

                return true;
            },
            [&property_name, &value](DBusMessage* reply) {
                std::cout << "[SET] " << property_name << " = " << value << " (success)" << std::endl;
            });
    }
};

//
// Main
//

int main() {
    DBusError error;
    DBusConnection* connection;

    dbus_error_init(&error);

    // Connect to the session bus
    connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Connection Error: " << error.message << std::endl;
        dbus_error_free(&error);
        return 1;
    }

    // Create client
    PropertyClient client(connection);

    std::cout << "=== Property Client ===" << std::endl;
    std::cout << std::endl;

    // Get initial values
    std::cout << "--- Getting Initial Properties ---" << std::endl;
    client.getIntProperty("Temperature");
    client.getIntProperty("Brightness");
    client.getStringProperty("DeviceName");
    client.getStringProperty("Status");
    std::cout << std::endl;

    // Set new values
    std::cout << "--- Setting New Properties ---" << std::endl;
    client.setIntProperty("Temperature", 30);
    client.setIntProperty("Brightness", 100);
    client.setStringProperty("DeviceName", "UpdatedDevice");
    client.setStringProperty("Status", "Running");
    std::cout << std::endl;

    // Get updated values
    std::cout << "--- Getting Updated Properties ---" << std::endl;
    client.getIntProperty("Temperature");
    client.getIntProperty("Brightness");
    client.getStringProperty("DeviceName");
    client.getStringProperty("Status");
    std::cout << std::endl;

    // Cleanup
    dbus_connection_unref(connection);

    return 0;
}
