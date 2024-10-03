#include <iostream>
#include <thread>
#include <future>       // for std::async
//#include <memory>       // for smart pointer

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_server_wrapper.hpp"

//
// Controller
//

class HelloController : public IRouter {
public:
    // Constructor
    HelloController() {}
    // delete
    HelloController(HelloController&& other) = delete;
    HelloController& operator=(HelloController&& other) = delete;
    HelloController(const HelloController&) = delete;
    HelloController& operator=(const HelloController&) = delete;
    // De-Constructor
    ~HelloController() {}

public:
    DBusMessage* handleRequest(DBusMessage* message) override {
        if ( true == dbus_message_is_method_call(message, "com.example.HelloInterface", "Hello") ) {
            return HelloController::prepareReply(message, HelloController::Hello);
        }
        else {
            return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found");
        }
    }

private:
    static DBusMessage* prepareReply(DBusMessage* message, std::function<std::string(const std::string&)> method) {
        // Get input from message (single input)
        const char* input;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_STRING,
            &input,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Invalid argument format");
        } 
                       
        // Call method
        std::string output = method(input);
        
        // Create a reply message from message
        DBusMessage* reply = dbus_message_new_method_return(message);

        // Append output to reply
        dbus_message_append_args(
            reply,
            DBUS_TYPE_STRING,
            &output,
            DBUS_TYPE_INVALID);
        
        // Return reply
        return reply;
    }

private:
    static std::string Hello(const std::string& name) {
        return "Hello " + name + "!\n";
    }
};

//
// Block Accept
//

class BlockAcceptService : public DBusServer {
public:
    // Constructor
    BlockAcceptService(DBusConnection* dc, HelloController* ctl) : DBusServer(dc, "com.example.HelloService", ctl) {}
    // delete
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
// Async Accept
//

class AsyncAcceptService : public DBusServer {
public:
    // Constructor
    AsyncAcceptService(DBusConnection* dc, HelloController* ctl) : DBusServer(dc, "com.example.HelloService", ctl) {}
    // delete
    AsyncAcceptService(AsyncAcceptService&& other) = delete;
    AsyncAcceptService& operator=(AsyncAcceptService&& other) = delete;
    AsyncAcceptService(const AsyncAcceptService&) = delete;
    AsyncAcceptService& operator=(const AsyncAcceptService&) = delete;
    // De-Constructor
    ~AsyncAcceptService() {}

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

            std::async(std::launch::async,
                [this, message] () {
                    // Run session
                    this->DBusServer::runSession(message);

                    // Release Message
                    if (message) {
                        dbus_message_unref(message);
                    }
                } );
        }
    }
};

//
// Thread Accept
//
// .detach() 容易有 client 收不到回覆的問題

class ThreadAcceptService : public DBusServer {
public:
    // Constructor
    ThreadAcceptService(DBusConnection* dc, HelloController* ctl) : DBusServer(dc, "com.example.HelloService", ctl) {}
    // delete
    ThreadAcceptService(ThreadAcceptService&& other) = delete;
    ThreadAcceptService& operator=(ThreadAcceptService&& other) = delete;
    ThreadAcceptService(const ThreadAcceptService&) = delete;
    ThreadAcceptService& operator=(const ThreadAcceptService&) = delete;
    // De-Constructor
    ~ThreadAcceptService() {}

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

            std::thread( [this, message] ()
                {
                    // Run session
                    DBusServer::runSession(message);

                    // Release Message
                    if (message) {
                        dbus_message_unref(message);
                    }
                } ).detach();
                //} ).join();
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
    HelloController hello_ctl;

    // Service
#if defined(BLOCK_ACCEPT)
    BlockAcceptService service(dbus_conn.getConn(), &hello_ctl);
#elif defined(ASYNC_ACCEPT)
    AsyncAcceptService service(dbus_conn.getConn(), &hello_ctl);
#elif defined(THREAD_ACCEPT)
    ThreadAcceptService service(dbus_conn.getConn(), &hello_ctl);
#else
    BlockAcceptService service(dbus_conn.getConn(), &hello_ctl);
#endif
    service.run();

    return 0;
}