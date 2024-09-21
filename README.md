# How to be client

https://github.com/makercrew/dbus-sample


$ apt-file search dbus/dbus.h
libdbus-1-dev: /usr/include/dbus-1.0/dbus/dbus.h
libdbus-cpp-dev: /usr/include/core/dbus/dbus.h 


#include <dbus/dbus.h>

#include <cstddef>
#include <cstdio>
#include <iostream>

int
main (
  int argc,
  char * argv[]
) {
    (void)argc;
    (void)argv;
    DBusError dbus_error;
    DBusConnection * dbus_conn = nullptr;
    DBusMessage * dbus_msg = nullptr;
    DBusMessage * dbus_reply = nullptr;
    const char * dbus_result = nullptr;

    // Initialize D-Bus error
    ::dbus_error_init(&dbus_error);

    // Connect to D-Bus
    if ( nullptr == (dbus_conn = ::dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error)) ) {
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);

    // Compose remote procedure call
    } else if ( nullptr == (dbus_msg = ::dbus_message_new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus.Introspectable", "Introspect")) ) {
        ::dbus_connection_unref(dbus_conn);
        ::perror("ERROR: ::dbus_message_new_method_call - Unable to allocate memory for the message!");

    // Invoke remote procedure call, block for response
    } else if ( nullptr == (dbus_reply = ::dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error)) ) {
        ::dbus_message_unref(dbus_msg);
        ::dbus_connection_unref(dbus_conn);
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);

    // Parse response
    } else if ( !::dbus_message_get_args(dbus_reply, &dbus_error, DBUS_TYPE_STRING, &dbus_result, DBUS_TYPE_INVALID) ) {
        ::dbus_message_unref(dbus_msg);
        ::dbus_message_unref(dbus_reply);
        ::dbus_connection_unref(dbus_conn);
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);

    // Work with the results of the remote procedure call
    } else {
        std::cout << "Connected to D-Bus as \"" << ::dbus_bus_get_unique_name(dbus_conn) << "\"." << std::endl;
        std::cout << "Introspection Result:" << std::endl << std::endl;
        std::cout << dbus_result << std::endl;
        ::dbus_message_unref(dbus_msg);
        ::dbus_message_unref(dbus_reply);

        /*
         * Applications must not close shared connections -
         * see dbus_connection_close() docs. This is a bug in the application.
         */
        //::dbus_connection_close(dbus_conn);

        // When using the System Bus, unreference
        // the connection instead of closing it
        ::dbus_connection_unref(dbus_conn);
    }

    return 0;
}

g++ dbus.cpp -std=c++0x $(pkg-config dbus-1 --cflags) -ldbus-1 -Werror -Wall -Wextra

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */


## if need to append argument before call method

// dbus_message_new_method_call ...

int arg = 42;  // Example argument
dbus_message_append_args(message, DBUS_TYPE_INT32, &arg, DBUS_TYPE_INVALID);

// after append ...

## dbus method

The service name org.freedesktop.DBus, the object path /org/freedesktop/DBus, and the interface org.freedesktop.DBus.Introspectable.

The Introspect method 

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
