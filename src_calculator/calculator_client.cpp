#include <iostream>
#include <functional>
#include <cstdlib>
#include <cstring>

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_client_wrapper.hpp"

class CalculatorClient : public DBusClient {
private:
    const char* service_name;
    const char* object_path;
    const char* interface_name;

public:
    // Constructor
    CalculatorClient(DBusConnection* dc):
        DBusClient(dc),
        service_name("com.example.CalcService"),
        object_path("/com/example/CalcService"),
        interface_name("com.example.CalcInterface")
        {}
    // delete
    CalculatorClient() = delete;
    CalculatorClient(CalculatorClient&& other) = delete;
    CalculatorClient& operator=(CalculatorClient&& other) = delete;
    CalculatorClient(const CalculatorClient&) = delete;
    CalculatorClient& operator=(const CalculatorClient&) = delete;
    // De-Constructor
    ~CalculatorClient() {}

public:
    // Add(int a, int b) -> int
    void callAdd(int a, int b, const std::function<void(int)>& callback = nullptr) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Add",
            [&a, &b] (DBusMessage* method_call) {
                if ( false == dbus_message_append_args(
                    method_call,
                    DBUS_TYPE_INT32,
                    &a,
                    DBUS_TYPE_INT32,
                    &b,
                    DBUS_TYPE_INVALID)
                    ) {
                    std::cerr << "ERROR: dbus_message_append_args - Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [this, &callback] (DBusMessage* reply) {
                this->CalculatorClient::parseAdd(reply, callback);
            }
        );
    }

    // Multiply(double a, double b) -> double
    void callMultiply(double a, double b, const std::function<void(double)>& callback = nullptr) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Multiply",
            [&a, &b] (DBusMessage* method_call) {
                if ( false == dbus_message_append_args(
                    method_call,
                    DBUS_TYPE_DOUBLE,
                    &a,
                    DBUS_TYPE_DOUBLE,
                    &b,
                    DBUS_TYPE_INVALID)
                    ) {
                    std::cerr << "ERROR: dbus_message_append_args - Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [this, &callback] (DBusMessage* reply) {
                this->CalculatorClient::parseMultiply(reply, callback);
            }
        );
    }

    // Concatenate(string s1, string s2) -> string
    void callConcatenate(const char* s1, const char* s2, const std::function<void(const std::string&)>& callback = nullptr) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "Concatenate",
            [&s1, &s2] (DBusMessage* method_call) {
                if ( false == dbus_message_append_args(
                    method_call,
                    DBUS_TYPE_STRING,
                    &s1,
                    DBUS_TYPE_STRING,
                    &s2,
                    DBUS_TYPE_INVALID)
                    ) {
                    std::cerr << "ERROR: dbus_message_append_args - Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [this, &callback] (DBusMessage* reply) {
                this->CalculatorClient::parseConcatenate(reply, callback);
            }
        );
    }

    // ProcessData(string name, int age, double salary) -> string
    void callProcessData(const char* name, int age, double salary, const std::function<void(const std::string&)>& callback = nullptr) {
        DBusClient::callMethod(
            this->service_name,
            this->object_path,
            this->interface_name,
            "ProcessData",
            [&name, &age, &salary] (DBusMessage* method_call) {
                if ( false == dbus_message_append_args(
                    method_call,
                    DBUS_TYPE_STRING,
                    &name,
                    DBUS_TYPE_INT32,
                    &age,
                    DBUS_TYPE_DOUBLE,
                    &salary,
                    DBUS_TYPE_INVALID)
                    ) {
                    std::cerr << "ERROR: dbus_message_append_args - Unable to append arguments!" << std::endl;
                    return false;
                }
                return true;
            },
            [this, &callback] (DBusMessage* reply) {
                this->CalculatorClient::parseProcessData(reply, callback);
            }
        );
    }

private:
    // Parse Add response
    void parseAdd(DBusMessage* reply, const std::function<void(int)>& callback) {
        DBusError error;
        dbus_error_init(&error);

        int result;
        if ( false == dbus_message_get_args(
            reply,
            &error,
            DBUS_TYPE_INT32,
            &result,
            DBUS_TYPE_INVALID)
            ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
            dbus_error_free(&error);
            return;
        }

        if (callback) {
            callback(result);
        }
        dbus_error_free(&error);
    }

    // Parse Multiply response
    void parseMultiply(DBusMessage* reply, const std::function<void(double)>& callback) {
        DBusError error;
        dbus_error_init(&error);

        double result;
        if ( false == dbus_message_get_args(
            reply,
            &error,
            DBUS_TYPE_DOUBLE,
            &result,
            DBUS_TYPE_INVALID)
            ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
            dbus_error_free(&error);
            return;
        }

        if (callback) {
            callback(result);
        }
        dbus_error_free(&error);
    }

    // Parse Concatenate response
    void parseConcatenate(DBusMessage* reply, const std::function<void(const std::string&)>& callback) {
        DBusError error;
        dbus_error_init(&error);

        const char* result;
        if ( false == dbus_message_get_args(
            reply,
            &error,
            DBUS_TYPE_STRING,
            &result,
            DBUS_TYPE_INVALID)
            ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
            dbus_error_free(&error);
            return;
        }

        if (callback) {
            callback(std::string(result));
        }
        dbus_error_free(&error);
    }

    // Parse ProcessData response
    void parseProcessData(DBusMessage* reply, const std::function<void(const std::string&)>& callback) {
        DBusError error;
        dbus_error_init(&error);

        const char* result;
        if ( false == dbus_message_get_args(
            reply,
            &error,
            DBUS_TYPE_STRING,
            &result,
            DBUS_TYPE_INVALID)
            ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
            dbus_error_free(&error);
            return;
        }

        if (callback) {
            callback(std::string(result));
        }
        dbus_error_free(&error);
    }
};

//
// main
//

int main() {
    // Use system bus by default.
    DBusBusType bus_type = DBUS_BUS_SYSTEM;
    const char* env_bus_type = std::getenv("DBUS_BUS_TYPE");
    if (env_bus_type && std::strcmp(env_bus_type, "session") == 0) {
        bus_type = DBUS_BUS_SESSION;
    }

    // DBus connection (SYSTEM by default; set DBUS_BUS_TYPE=session to override)
    DBusConn dbus_conn(bus_type);

    // Client
    CalculatorClient client(dbus_conn.getConn());

    std::cout << "=== Calculator Service Client ===" << std::endl << std::endl;

    // Test Add(5, 3) -> 8
    std::cout << "Calling Add(5, 3)..." << std::endl;
    client.callAdd(5, 3, [](int result) {
        std::cout << "Result: " << result << std::endl;
    });

    std::cout << std::endl;

    // Test Multiply(2.5, 4.0) -> 10.0
    std::cout << "Calling Multiply(2.5, 4.0)..." << std::endl;
    client.callMultiply(2.5, 4.0, [](double result) {
        std::cout << "Result: " << result << std::endl;
    });

    std::cout << std::endl;

    // Test Concatenate("Hello", "World") -> "Hello + World"
    std::cout << "Calling Concatenate(\"Hello\", \"World\")..." << std::endl;
    client.callConcatenate("Hello", "World", [](const std::string& result) {
        std::cout << "Result: " << result << std::endl;
    });

    std::cout << std::endl;

    // Test ProcessData("John", 30, 50000.50) -> employee info
    std::cout << "Calling ProcessData(\"John\", 30, 50000.50)..." << std::endl;
    client.callProcessData("John", 30, 50000.50, [](const std::string& result) {
        std::cout << "Result: " << result << std::endl;
    });

    return 0;
}
