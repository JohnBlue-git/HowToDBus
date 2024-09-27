/*

g++ hello_client.cpp -o hello_client $(pkg-config dbus-1 --cflags) -ldbus-1 -lpthread -Wall -Wextra

*/

#include <iostream>

#include "../include/dbus_wrapper.hpp"

class HelloClient : public DBusClient {
private:
    const char* service_name;// const no need to free
    const char* object_path;// const no need to free
    const char* interface_name;// const no need to free
    const char* response = nullptr;
    const char* who = nullptr;

public:
    // Constructor
    HelloClient(DBusConnection* dc, const char* nm):
        DBusClient(dc),
        service_name("com.example.HelloService"),
        object_path("/com/example/HelloService"),
        interface_name("com.example.HelloInterface"),
        who(nm)
        {}
    // delete
    HelloClient(HelloClient&& other) = delete;
    HelloClient& operator=(HelloClient&& other) = delete;
    HelloClient(const HelloClient&) = delete;
    HelloClient& operator=(const HelloClient&) = delete;
    // De-Constructor
    ~HelloClient() {}

public:
    void callHello() {
        // Connection
        if (! this->conn) {
            return;
        }

        // Compose remote procedure call
        DBusMessage* method_call = DBusClient::composeMethodCall(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Hello"
        );
        if (! method_call) {
            return;
        }

        // Append argument
        dbus_message_append_args(method_call, DBUS_TYPE_STRING, &who, DBUS_TYPE_INVALID);

        // Send method call
        DBusMessage* reply = DBusClient::sendMethodCall(method_call);
        // Parse response and Store
        HelloClient::parseHello(reply);
        // Release reply
        dbus_message_unref(reply);

        // Execute callback
    }

public:
    void showHello() {
        std::cout << response << std::endl;
    }

private:
    // s
    void parseHello(DBusMessage* reply) {
        if ( ! dbus_message_get_args(reply, &(this->error), DBUS_TYPE_STRING, &(this->response), DBUS_TYPE_INVALID) ) {
            std::cout << this->error.name << std::endl << this->error.message << std::endl;
        }
    }
};

int main() {
    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    HelloClient client(dbus_conn.getConn(), "World");
    client.callHello();
    client.showHello();

    return 0;
}