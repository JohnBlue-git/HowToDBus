/*

g++ ../hello_client.cpp -o hello_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

*/

#include <iostream>
#include <functional>

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_client_wrapper.hpp"

class HelloClient : public DBusClient {
private:
    const char* service_name;// const no need to free
    const char* object_path;// const no need to free
    const char* interface_name;// const no need to free

public:
    // Constructor
    HelloClient(DBusConnection* dc):
        DBusClient(dc),
        service_name("com.example.HelloService"),
        object_path("/com/example/HelloService"),
        interface_name("com.example.HelloInterface")
        {}
    // delete
    HelloClient(HelloClient&& other) = delete;
    HelloClient& operator=(HelloClient&& other) = delete;
    HelloClient(const HelloClient&) = delete;
    HelloClient& operator=(const HelloClient&) = delete;
    // De-Constructor
    ~HelloClient() {}

public:
    static void showHello(const std::string& response) {
        std::cout << response << std::endl;
    }

public:
    void callHello(const char* who, const std::function<void(const std::string&)>& callback = nullptr) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Hello",
            [&who] (DBusMessage* method_call) {
                if ( false == dbus_message_append_args(
                    method_call,
                    DBUS_TYPE_STRING,
                    &who,
                    DBUS_TYPE_INVALID)
                    ) {
                    std::cerr << "ERROR: dbus_message_append_args - Unable to append argument!" << std::endl;
                    return false;
                }
                return true;
            },
            [this, &callback] (DBusMessage* reply) {
                this->HelloClient::parseHello(reply, callback);
            }
        );
    }

private:
    // s
    void parseHello(DBusMessage* reply, const std::function<void(const std::string&)>& callback) {
        DBusError error;
        dbus_error_init(&error);

        // response
        const char* response;

        // Parse reply
        if ( false == dbus_message_get_args(
            reply,
            &error,
            DBUS_TYPE_STRING,
            &response,
            DBUS_TYPE_INVALID)
            ) {
            std::cout << error.name << std::endl << error.message << std::endl;
        }

        // Execute callback
        if (callback) {
            callback(response);
        }

        dbus_error_free(&error);
    }
};

int main() {
    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    HelloClient client(dbus_conn.getConn());
    // static function
    client.callHello("World", HelloClient::showHello);
    // lambda function
    client.callHello(
        "World",
        [] (const std::string& response) {
            std::cout << response << std::endl;
        }
    );

    return 0;
}