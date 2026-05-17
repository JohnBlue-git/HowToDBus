#include <atomic>
#include <csignal>
#include <cstdint>
#include <iostream>

#include "../include/dbus_conn_wrapper.hpp"

namespace {
constexpr const char* kInterfaceName = "com.example.SignalInterface";
constexpr const char* kSignalName = "Tick";

std::atomic<bool> running{true};

void handleSignal(int) {
    running.store(false);
}
}

int main() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    DBusConn dbusConn(DBUS_BUS_SESSION);
    DBusConnection* conn = dbusConn.getConn();
    if (conn == nullptr) {
        return 1;
    }

    DBusError error;
    dbus_error_init(&error);

    const char* matchRule = "type='signal',interface='com.example.SignalInterface',member='Tick'";
    dbus_bus_add_match(conn, matchRule, &error);
    dbus_connection_flush(conn);

    if (dbus_error_is_set(&error)) {
        std::cerr << "Match rule error: " << error.message << std::endl;
        dbus_error_free(&error);
        return 1;
    }

    std::cout << "Listening signal" << std::endl;
    std::cout << "Interface: " << kInterfaceName << std::endl;
    std::cout << "Signal: " << kSignalName << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    while (running.load()) {
        dbus_connection_read_write(conn, 1000);

        DBusMessage* message = dbus_connection_pop_message(conn);
        if (message == nullptr) {
            continue;
        }

        if (dbus_message_is_signal(message, kInterfaceName, kSignalName)) {
            uint32_t counter = 0;
            const char* text = "";

            if (dbus_message_get_args(message,
                                      &error,
                                      DBUS_TYPE_UINT32,
                                      &counter,
                                      DBUS_TYPE_STRING,
                                      &text,
                                      DBUS_TYPE_INVALID)) {
                std::cout << "[RECV] Tick counter=" << counter << ", text=" << text << std::endl;
            } else {
                std::cerr << "Parse signal error: " << error.message << std::endl;
                dbus_error_free(&error);
                dbus_error_init(&error);
            }
        }

        dbus_message_unref(message);
    }

    dbus_error_free(&error);
    std::cout << "Signal client stopped" << std::endl;
    return 0;
}
