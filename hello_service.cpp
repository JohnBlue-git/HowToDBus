#include <iostream>
#include <string.h>
#include <unistd.h>  // Include this header for usleep

#include <dbus/dbus.h>

// Define the method implementation
std::string Hello() {
    return "Hello, D-Bus!";
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    // Connect to the session bus
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Connection Error: " << error.message << std::endl;
        dbus_error_free(&error);
        return 1;
    }

    // Request a name on the bus
    int ret = dbus_bus_request_name(connection, "com.example.HelloService", DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Name Error: " << error.message << std::endl;
        dbus_error_free(&error);
        return 1;
    }
    
    // Main loop
    while (true) {
        // Handle incoming messages
        dbus_connection_read_write(connection, 0);
        
        DBusMessage* message = dbus_connection_pop_message(connection);
        if (message) {
            if (dbus_message_is_method_call(message, "com.example.HelloService", "Hello")) {
                // Create a reply message
                DBusMessage* reply = dbus_message_new_method_return(message);
                std::string greeting = Hello();
                dbus_message_append_args(reply, DBUS_TYPE_STRING, &greeting, DBUS_TYPE_INVALID);
                
                // Send the reply
                dbus_connection_send(connection, reply, nullptr);
                dbus_message_unref(reply);
            }
            dbus_message_unref(message);
        }
        
        // Sleep briefly to avoid busy waiting
        usleep(100000); // 100 ms
    }

    dbus_connection_unref(connection);
    return 0;
}