/*

Configuration Location:
/etc/dbus-1/system.d/ for system services
/etc/dbus-1/session.d/ for session services

com.example.HelloService.conf

<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
    <!--   *   Allow any user to talk to our service -->
    <!-- $USER Replace with your username -->
    <policy user="*">
        <allow own="com.example.HelloService"/>
        <allow send_destination="com.example.HelloService"/>
        <allow receive_sender="com.example.HelloService"/>
        <allow send_interface="com.example.HelloInterface"/>
    </policy>
</busconfig>

# <thread>, <future>, async related,  need -lpthread
g++ hello_service.cpp -o hello_service $(pkg-config dbus-1 --cflags) -ldbus-1 -lpthread -Wall -Wextra  -DBLOCK_ACCEPT
 -DBLOCK_ACCEPT

./hello_service
sudo ./hello_service will trigger many thing that are not allowed and show error message

busctl call --system com.example.HelloService /com/example/HelloService com.example.HelloInterface Hello s world

*/

#include <iostream>
#include <thread>
#include <future>       // for std::async

#include "../include/dbus_wrapper.hpp"


class HelloService {
private:
    DBusError error;
private:
    DBusConnection* conn;
    const char* service_name;// const no need to free
    bool runnable = true;

public:
    // Constructor
    HelloService(DBusConnection* dc):
        conn(dc),
        service_name("com.example.HelloService")
    {
        dbus_error_init(&error);

        // Request a name on the bus
        if ( ! dbus_bus_request_name(this->conn, this->service_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &(this->error)) ) {
            runnable = false;
            std::cerr << "Name Error: " << this->error.message << std::endl;
        }
    }
    // delete
    HelloService(HelloService&& other) = delete;
    HelloService& operator=(HelloService&& other) = delete;
    HelloService(const HelloService&) = delete;
    DBusClient& operator=(const HelloService&) = delete;
    // De-Constructor
    ~HelloService() {
        dbus_error_free(&error);
    }

public:
    void run_blocking_accept() {
        // Main loop
        while (true) {
            // Without blocking (drawback is busy waiting)
            //dbus_connection_read_write(this->conn, 1000);
            // will return true/false; can set timeout (ms)
            // should accompany with
            //#include <unistd.h>
            //usleep(1000);

            // Blocking way
            // useful to look at
            // spec: https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga580d8766c23fe5f49418bc7d87b67dc6
            dbus_connection_read_write_dispatch(this->conn, -1);

            // Initialize variables here to avoid crosses initialization (related to goto)
            DBusMessage* message = nullptr;
            DBusMessage* reply = nullptr;

            // Get message
            if ( nullptr == (message = dbus_connection_pop_message(this->conn)) ) {
                std::cout << this->error.name << std::endl << this->error.message << std::endl;
                goto UNREF_MESSAGE;
            }

            // Perform the task asynchronously
            /*
            std::async(std::launch::async, 
                [] () {
                    //
                    std::cerr << "async: " << std::endl;
                });
            */

            // Check request
            if ( true == dbus_message_is_method_call(message, "com.example.HelloInterface", "Hello") ) {
                reply = HelloService::createReplyFromHello(message);
            }
            else {
                goto UNREF_MESSAGE;
            }

            // Send the reply
            if (reply) {
                dbus_connection_send(this->conn, reply, nullptr);
                // Release reply
                dbus_message_unref(reply);
            }

    UNREF_MESSAGE:
            if (message)
                dbus_message_unref(message);
        }
    }
    
    void run_accept_then_threading() {}
    
    void run_async_accept() {}

private:
    DBusMessage* createReplyFromHello(DBusMessage* message) {
        // Get input
        const char* inputName;
        dbus_message_get_args(message, nullptr, DBUS_TYPE_STRING, &inputName, DBUS_TYPE_INVALID);
                        
        // Hello method
        std::string greeting = HelloService::Hello(inputName);
        
        // Create a reply message
        DBusMessage* reply = dbus_message_new_method_return(message);
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &greeting, DBUS_TYPE_INVALID);
        // not yet DBusClient::appendArg(reply, DBUS_TYPE_STRING, "hello");//&greeting
        return reply;
    }

private:
    // Hello method implementation
    std::string Hello(const std::string& name) {
        return "Hello " + name + "!\n";
    }
}; 

int main() {
    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    // Service
    HelloService service(dbus_conn.getConn());
#ifdef BLOCK_ACCEPT
    service.run_blocking_accept();
#endif
#ifdef ASYNC_ACCEPT
    service.run_async_accept();
#endif
#ifdef THREAD_ACCEPT
    service.run_accept_then_threading();
#endif

    return 0;
}