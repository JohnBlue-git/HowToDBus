# How to be client

https://github.com/makercrew/dbus-sample

# How to be service

Step 1: Set Up Your Environment
Make sure you have the D-Bus development libraries installed. On most Linux systems, you can install them using your package manager. For example:

sudo apt-get install libdbus-1-dev
Step 2: Define Your Service Interface
You’ll need to define the interface for your service. For example, let’s create a service with one method called Hello that returns a greeting.

Step 3: Implement the D-Bus Service
Here’s a simple example of a D-Bus service in C++:

#include <dbus/dbus.h>
#include <iostream>
#include <string.h>

// Define the method implementation
std::string Hello() {
    return "Hello, D-Bus!";
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    // Connect to the session bus
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
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

Step 4: Build the Service
Compile your service with the D-Bus library:

g++ -o hello_service hello_service.cpp `pkg-config --cflags --libs dbus-1`
Step 5: Run the Service
Start your D-Bus service in a terminal:

./hello_service
Step 6: Test the Service
You can test your service using gdbus or dbus-send. For example, to call the Hello method:

gdbus call --session --dest com.example.HelloService --object-path /com/example/HelloService --method com.example.HelloService.Hello
Summary
This example demonstrates how to create a simple D-Bus service in C++. You:

Set up the environment and libraries.
Defined an interface with a method.
Implemented the method and registered the service with D-Bus.
Built and ran the service.
