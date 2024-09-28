#include <iostream>
#include <thread>
#include <future>       // for std::async

#include "../include/dbus_conn_wrapper.hpp"
#include "../include/dbus_server_wrapper.hpp"

class HelloService : public DBusServer {
private:
    DBusError error;
private:
    const char* service_name;// const no need to free
    bool runnable = true;

public:
    // Constructor
    HelloService(DBusConnection* dc):
        DBusServer(dc),
        service_name("com.example.HelloService")
    {
        dbus_error_init(&error);

        // Request a name on the bus
        if ( false == dbus_bus_request_name(this->conn, this->service_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &(this->error)) ) {
            runnable = false;
            std::cerr << "Name Error: " << this->error.message << std::endl;
        }
    }
    // delete
    HelloService(HelloService&& other) = delete;
    HelloService& operator=(HelloService&& other) = delete;
    HelloService(const HelloService&) = delete;
    HelloService& operator=(const HelloService&) = delete;
    // De-Constructor
    ~HelloService() {
        dbus_error_free(&error);
    }

public:
    void run_blocking_accept() {
        if (false == runnable) {
            return;
        }
        runnable = true;

        DBusServer::run(
            [this] () {
                this->DBusServer::handleRequest(
                    [this] (DBusMessage* message) {
                        if ( true == dbus_message_is_method_call(message, "com.example.HelloInterface", "Hello") ) {
                            return this->HelloService::createReplyFromHello(message);
                        }
                        else {
                            return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found");
                        }
                    }
                );
            }
        );
    }
    
    // !!! 可能會有 pointer 掛掉的問題
    //     再找時間去 smart pointer 處理 ？
    void run_accept_then_threading() {
        if (false == runnable) {
            return;
        }
        runnable = true;

        DBusServer::run(
            [this] () {
                std::thread(
                    [this] () {
                        this->DBusServer::handleRequest(
                            [this] (DBusMessage* message) {
                                if ( true == dbus_message_is_method_call(message, "com.example.HelloInterface", "Hello") ) {
                                    return this->HelloService::createReplyFromHello(message);
                                }
                                else {
                                    return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_METHOD, "Method not found");
                                }
                            }
                        );
                    } ).detach();
            }
        );
    }
    
    void run_async_accept() {
        if (false == runnable) {
            return;
        }
        runnable = true;

            // Perform the task asynchronously
            /*
            std::async(std::launch::async, 
                [] () {
                    //
                    std::cerr << "async: " << std::endl;
                });
            */
    }

private:
    DBusMessage* createReplyFromHello(DBusMessage* message) {
        // Get input
        const char* inputName;
        if ( false == dbus_message_get_args(
            message,
            nullptr,
            DBUS_TYPE_STRING,
            &inputName,
            DBUS_TYPE_INVALID)
            ) {
            return dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "Invalid argument format");
        } 
                       
        // Hello method
        std::string greeting = HelloService::Hello(inputName);
        
        // Create a reply message
        //
        DBusMessage* reply = dbus_message_new_method_return(message);
        //
        dbus_message_append_args(
            reply,
            DBUS_TYPE_STRING,
            &greeting,
            DBUS_TYPE_INVALID);
        //
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