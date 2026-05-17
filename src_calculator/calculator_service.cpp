#include <iostream>
#include <thread>
#include <future>
#include <cstdlib>
#include <cstring>

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_server_wrapper.hpp"

//
// Controller
//

class CalculatorController : public IRouter {
public:
    // Constructor
    CalculatorController() {}
    // delete
    CalculatorController(CalculatorController&& other) = delete;
    CalculatorController& operator=(CalculatorController&& other) = delete;
    CalculatorController(const CalculatorController&) = delete;
    CalculatorController& operator=(const CalculatorController&) = delete;
    // De-Constructor
    ~CalculatorController() {}

public:
    DBusMessage* handleRequest(DBusMessage* message) override {
        // Add(int a, int b) -> int result
        if ( true == dbus_message_is_method_call(message, "com.example.CalcInterface", "Add") ) {
            return CalculatorController::prepareReplyAdd(message);
        }
        // Multiply(double a, double b) -> double result
        else if ( true == dbus_message_is_method_call(message, "com.example.CalcInterface", "Multiply") ) {
            return CalculatorController::prepareReplyMultiply(message);
        }
        // Concatenate(string s1, string s2) -> string result
        else if ( true == dbus_message_is_method_call(message, "com.example.CalcInterface", "Concatenate") ) {
            return CalculatorController::prepareReplyConcatenate(message);
        }
        // ProcessData(string name, int age, double salary) -> string message
        else if ( true == dbus_message_is_method_call(message, "com.example.CalcInterface", "ProcessData") ) {
            return CalculatorController::prepareReplyProcessData(message);
        }
        else {
            return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found");
        }
    }

private:
    // Add(int a, int b) -> int
    static DBusMessage* prepareReplyAdd(DBusMessage* message) {
        int a, b;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_INT32,
            &a,
            DBUS_TYPE_INT32,
            &b,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Expected two int32 arguments");
        }

        int result = a + b;
        DBusMessage* reply = dbus_message_new_method_return(message);
        dbus_message_append_args(
            reply,
            DBUS_TYPE_INT32,
            &result,
            DBUS_TYPE_INVALID);
        
        return reply;
    }

    // Multiply(double a, double b) -> double
    static DBusMessage* prepareReplyMultiply(DBusMessage* message) {
        double a, b;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_DOUBLE,
            &a,
            DBUS_TYPE_DOUBLE,
            &b,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Expected two double arguments");
        }

        double result = a * b;
        DBusMessage* reply = dbus_message_new_method_return(message);
        dbus_message_append_args(
            reply,
            DBUS_TYPE_DOUBLE,
            &result,
            DBUS_TYPE_INVALID);
        
        return reply;
    }

    // Concatenate(string s1, string s2) -> string
    static DBusMessage* prepareReplyConcatenate(DBusMessage* message) {
        const char* s1;
        const char* s2;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_STRING,
            &s1,
            DBUS_TYPE_STRING,
            &s2,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Expected two string arguments");
        }

        std::string result = std::string(s1) + " + " + std::string(s2);
        const char* result_cstr = result.c_str();
        DBusMessage* reply = dbus_message_new_method_return(message);
        dbus_message_append_args(
            reply,
            DBUS_TYPE_STRING,
            &result_cstr,
            DBUS_TYPE_INVALID);
        
        return reply;
    }

    // ProcessData(string name, int age, double salary) -> string
    static DBusMessage* prepareReplyProcessData(DBusMessage* message) {
        const char* name;
        int age;
        double salary;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_STRING,
            &name,
            DBUS_TYPE_INT32,
            &age,
            DBUS_TYPE_DOUBLE,
            &salary,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Expected string, int32, and double arguments");
        }

        std::string msg = std::string("Employee: ") + name + 
                         ", Age: " + std::to_string(age) + 
                         ", Salary: $" + std::to_string(salary);
        const char* msg_cstr = msg.c_str();
        DBusMessage* reply = dbus_message_new_method_return(message);
        dbus_message_append_args(
            reply,
            DBUS_TYPE_STRING,
            &msg_cstr,
            DBUS_TYPE_INVALID);
        
        return reply;
    }
};

//
// Block Accept Service (only version requested)
//

class BlockAcceptService : public DBusServer {
public:
    // Constructor
    BlockAcceptService(DBusConnection* dc, CalculatorController* ctl) : DBusServer(dc, "com.example.CalcService", ctl) {}
    // delete
    BlockAcceptService() = delete;
    BlockAcceptService(BlockAcceptService&& other) = delete;
    BlockAcceptService& operator=(BlockAcceptService&& other) = delete;
    BlockAcceptService(const BlockAcceptService&) = delete;
    BlockAcceptService& operator=(const BlockAcceptService&) = delete;
    // De-Constructor
    ~BlockAcceptService() {}

    void run() override
    {
        // Check runnable
        if (false == runnable) {
            return;
        }

        // Main loop
        for (;;) {
            // Wait for client connection
            dbus_connection_read_write_dispatch(this->conn, -1);

            // Get message
            DBusMessage* message = nullptr;
            if ( nullptr == (message = dbus_connection_pop_message(this->conn)) ) {
                std::cout << this->error.name << std::endl << this->error.message << std::endl;
                continue;
            }

            // Run session
            this->DBusServer::runSession(message);

            // Release Message
            if (message) {
                dbus_message_unref(message);
            }
        }
    }
};

//
// main
//

int main() {
    // DBus connection (type: DBUS_BUS_SYSTEM / DBUS_BUS_SESSION)
    DBusConn dbus_conn(DBUS_BUS_SYSTEM);

    // Controller
    CalculatorController calc_ctl;

    // Service (BlockAcceptService only)
    BlockAcceptService service(dbus_conn.getConn(), &calc_ctl);
    service.run();

    return 0;
}
