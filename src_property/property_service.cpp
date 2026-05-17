#include <iostream>
#include <map>
#include <thread>

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_server_wrapper.hpp"

//
// Property Storage
//

class PropertyStorage {
private:
    std::map<std::string, int32_t> int_properties;
    std::map<std::string, std::string> string_properties;

public:
    PropertyStorage() {
        // Initialize with default values
        int_properties["Temperature"] = 25;
        int_properties["Brightness"] = 80;
        string_properties["DeviceName"] = "PropertyDevice";
        string_properties["Status"] = "Ready";
    }

    bool getIntProperty(const std::string& name, int32_t& value) {
        auto it = int_properties.find(name);
        if (it != int_properties.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    bool setIntProperty(const std::string& name, int32_t value) {
        int_properties[name] = value;
        return true;
    }

    bool getStringProperty(const std::string& name, std::string& value) {
        auto it = string_properties.find(name);
        if (it != string_properties.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    bool setStringProperty(const std::string& name, const std::string& value) {
        string_properties[name] = value;
        return true;
    }

    void listProperties() const {
        std::cout << "Integer Properties: ";
        for (const auto& p : int_properties) {
            std::cout << p.first << "=" << p.second << " ";
        }
        std::cout << std::endl;
        
        std::cout << "String Properties: ";
        for (const auto& p : string_properties) {
            std::cout << p.first << "=" << p.second << " ";
        }
        std::cout << std::endl;
    }
};

//
// Controller
//

class PropertyController : public IRouter {
private:
    PropertyStorage properties;

public:
    // Constructor
    PropertyController() {}
    // delete
    PropertyController(PropertyController&& other) = delete;
    PropertyController& operator=(PropertyController&& other) = delete;
    PropertyController(const PropertyController&) = delete;
    PropertyController& operator=(const PropertyController&) = delete;
    // De-Constructor
    ~PropertyController() {}

public:
    DBusMessage* handleRequest(DBusMessage* message) override {
        // Handle Get property request
        if (true == dbus_message_is_method_call(message, "org.freedesktop.DBus.Properties", "Get")) {
            return handleGetProperty(message);
        }
        // Handle Set property request
        else if (true == dbus_message_is_method_call(message, "org.freedesktop.DBus.Properties", "Set")) {
            return handleSetProperty(message);
        }
        // Handle GetAll properties request
        else if (true == dbus_message_is_method_call(message, "org.freedesktop.DBus.Properties", "GetAll")) {
            return handleGetAllProperties(message);
        }
        else {
            return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found");
        }
    }

private:
    DBusMessage* handleGetProperty(DBusMessage* message) {
        DBusError error;
        dbus_error_init(&error);
        
        const char* interface_name;
        const char* property_name;
        
        if (!dbus_message_get_args(
                message,
                &error,
                DBUS_TYPE_STRING, &interface_name,
                DBUS_TYPE_STRING, &property_name,
                DBUS_TYPE_INVALID)) {
            DBusMessage* reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Invalid arguments");
            dbus_error_free(&error);
            return reply;
        }

        DBusMessage* reply = dbus_message_new_method_return(message);
        DBusMessageIter iter, variant_iter;
        
        dbus_message_iter_init_append(reply, &iter);

        int32_t int_value;
        std::string str_value;

        if (properties.getIntProperty(property_name, int_value)) {
            dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "i", &variant_iter);
            dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_INT32, &int_value);
            dbus_message_iter_close_container(&iter, &variant_iter);
        }
        else if (properties.getStringProperty(property_name, str_value)) {
            const char* str_ptr = str_value.c_str();
            dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
            dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &str_ptr);
            dbus_message_iter_close_container(&iter, &variant_iter);
        }
        else {
            dbus_message_unref(reply);
            dbus_error_free(&error);
            return dbus_message_new_error(message, "org.freedesktop.DBus.Error.PropertyNotFound", "Property not found");
        }

        dbus_error_free(&error);
        return reply;
    }

    DBusMessage* handleSetProperty(DBusMessage* message) {
        DBusError error;
        dbus_error_init(&error);
        
        const char* interface_name;
        const char* property_name;
        DBusMessageIter iter, variant_iter;
        int variant_type;
        
        if (!dbus_message_iter_init(message, &iter)) {
            DBusMessage* reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Invalid arguments");
            dbus_error_free(&error);
            return reply;
        }

        dbus_message_iter_get_basic(&iter, &interface_name);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &property_name);
        dbus_message_iter_next(&iter);

        dbus_message_iter_recurse(&iter, &variant_iter);
        variant_type = dbus_message_iter_get_arg_type(&variant_iter);

        if (variant_type == DBUS_TYPE_INT32) {
            int32_t value;
            dbus_message_iter_get_basic(&variant_iter, &value);
            if (properties.setIntProperty(property_name, value)) {
                std::cout << "Property set: " << property_name << " = " << value << std::endl;
                properties.listProperties();
                dbus_error_free(&error);
                return dbus_message_new_method_return(message);
            }
        }
        else if (variant_type == DBUS_TYPE_STRING) {
            const char* value;
            dbus_message_iter_get_basic(&variant_iter, &value);
            if (properties.setStringProperty(property_name, value)) {
                std::cout << "Property set: " << property_name << " = " << value << std::endl;
                properties.listProperties();
                dbus_error_free(&error);
                return dbus_message_new_method_return(message);
            }
        }

        dbus_error_free(&error);
        return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Unsupported property type");
    }

    DBusMessage* handleGetAllProperties(DBusMessage* message) {
        // For simplicity, this is a basic implementation
        DBusMessage* reply = dbus_message_new_method_return(message);
        DBusMessageIter iter, array_iter;

        dbus_message_iter_init_append(reply, &iter);
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array_iter);
        dbus_message_iter_close_container(&iter, &array_iter);

        return reply;
    }
};

//
// Service
//

class PropertyService : public DBusServer {
public:
    PropertyService(DBusConnection* dc, PropertyController* ctl) 
        : DBusServer(dc, "com.example.PropertyService", ctl) {}

    void run() override {
        std::cout << "Property Service started..." << std::endl;
        std::cout << "Service: com.example.PropertyService" << std::endl;
        std::cout << "Object Path: /com/example/PropertyService" << std::endl;
        std::cout << "Interface: org.freedesktop.DBus.Properties" << std::endl;
        std::cout << std::endl;

        DBusServer::run();
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

    // Create controller and service
    PropertyController controller;
    PropertyService service(connection, &controller);

    // Run service
    service.run();

    // Cleanup
    dbus_connection_unref(connection);

    return 0;
}
