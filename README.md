# How to be client

https://github.com/makercrew/dbus-sample

$ sudo apt install apt-file

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

g++ dbus_sample.cpp -o dbus_sample.cpp -std=c++0x $(pkg-config dbus-1 --cflags) -ldbus-1 -Werror -Wall -Wextra



/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */


## if need to append argument before call method

// dbus_message_new_method_call ...

int arg = 42;  // Example argument
dbus_message_append_args(message, DBUS_TYPE_INT32, &arg, DBUS_TYPE_INVALID);

// after append ...

## dbus method

The service name org.freedesktop.DBus, the object path /org/freedesktop/DBus, and the interface org.freedesktop.DBus.Introspectable.

The Introspect method 

## gdbus call

gdbus call --session --dest=org.freedesktop.DBus --object-path /org/freedesktop/DBus --method org.freedesktop.DBus.Introspectable.Introspect

## dbus send

dbus-send --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.Introspectable.Introspect

## dbus start (may bit start in Linux)

$ sudo systemctl start dbus
"systemd" is not running in this container due to its overhead.
Use the "service" command to start services instead. e.g.: 

$ service
Usage: service < option > | --status-all | [ service_name [ command | --full-restart ] ]
$ service --status-all
 [ ? ]  binfmt-support
 [ - ]  dbus
 [ ? ]  hwclock.sh
 [ - ]  procps
 [ - ]  rsync
 [ + ]  ssh
 [ - ]  x11-common
$ sudo service dbus --full-restart

## dbus session

System or Session ?

System D-Bus

Global Scope: The system D-Bus operates at the system level, meaning it is accessible to all users. This allows different user applications to communicate with system services, regardless of which user account they are running under.

Service Accessibility: Services that register with the system D-Bus are available to any user who has the appropriate permissions. For example, system services like network management or hardware access typically run on the system bus.

Access Control: Access to specific services on the system bus is controlled by policy files (usually found in /etc/dbus-1/system.d/). These files define who can call which methods, providing a layer of security.

User-Specific D-Bus

In contrast, each user account has its own session D-Bus, which is isolated and only accessible by processes running in that user’s session. This allows user-specific applications to communicate without affecting others.

$ ps -ef | grep dbus
message+    8172       1  0 12:52 ?        00:00:00 /usr/bin/dbus-daemon --system
codespa+   12543    4277  0 13:02 pts/2    00:00:00 dbus-run-session -- bash
codespa+   12544   12543  0 13:02 pts/2    00:00:00 dbus-daemon --nofork --print-address 4 --session

Why session ?

1. Isolation of Services
A D-Bus session creates an isolated environment for communication between applications. Each user session can have its own D-Bus session, which helps prevent interference between services running for different users.

2. Access Control
D-Bus implements a permission model where services can restrict access based on the session. By starting a new session, you ensure that only processes within that session can communicate with each other, enhancing security.

3. User-Specific Services
Many applications (especially desktop applications) register services on the session bus that are relevant only to the user currently logged in. Starting a D-Bus session ensures that you can interact with these user-specific services.

4. Environment Setup
A D-Bus session initializes necessary environment variables (like DBUS_SESSION_BUS_ADDRESS) that applications rely on to connect to the session bus. Without this, applications may fail to find and connect to the bus.

5. Development and Testing
For developers, starting a D-Bus session allows for testing applications in a controlled environment, simulating how they would operate in a full user session.

6. Access to Introspection and Signals
When a D-Bus session is active, you can leverage features like introspection (discovering available interfaces and methods) and signals (event notifications) effectively.

dbus related to session ?

dbus-launch (but not all linux have it)

dbus-run-session (can be used as a replacement)

dbus-run-session ?

If you want to run a terminal within a new D-Bus session, you can do:

dbus-run-session -- gnome-terminal

If you just want to interact with D-Bus commands, you can start a session and then run your commands in that session:

dbus-run-session -- bash

How to check session ?

echo $DBUS_SESSION_BUS_ADDRESS

How to create different session ?

dbus-run-session -- ./hello_service.sh &
dbus-run-session -- ./hello_service.sh &

How to distinquish ?

1. Use Environment Variables
When you start a session, you can export the address to a file or print it:
Start a new session and save the address

dbus-run-session -- bash -c 'echo $DBUS_SESSION_BUS_ADDRESS > /tmp/session1_address.txt && sleep 10'

2. Check Session Addresses
cat /tmp/session1_address.txt

3. Bind to a Specific Session
export DBUS_SESSION_BUS_ADDRESS=$(cat /tmp/session1_address.txt)







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

g++ hello_service.cpp -o hello_service $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

// -Werror
// will error about unused variable

Step 5: Run the Service
Start your D-Bus service in a terminal:

./hello_service

Step 6: Test the Service
You can test your service using gdbus or dbus-send. For example, to call the Hello method:


gdbus call --system --dest com.example.HelloService --object-path /com/example/HelloService --method com.example.HelloService.Hello


among Step 4 ~ 6: allow service

The error you're encountering indicates that your D-Bus service is being denied the ability to own the specified service name (com.example.HelloService) due to security policies defined in the D-Bus configuration files.

Create or Modify a D-Bus policy file that grants permission for your application to own the service name.

Location:
/etc/dbus-1/system.d/ for system services
/etc/dbus-1/session.d/ for session services.

Create a file called com.example.HelloService.conf:

<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
    <policy user="yourusername">  <!-- Replace with your username -->
        <allow own="com.example.HelloService"/>
        <allow send_destination="com.example.HelloService"/>
        <allow receive_sender="com.example.HelloService"/>
    </policy>
</busconfig>

Make sure to replace yourusername with your actual username.
Restart D-Bus: After modifying or creating the policy file, you may need to restart the D-Bus service to apply the changes.
For session bus, you can usually just log out and log back in. For system bus, you may use:

sudo systemctl restart dbus
