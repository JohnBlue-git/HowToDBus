#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <thread>

#include "../include/dbus_conn_wrapper.hpp"

namespace {
constexpr const char* kServiceName = "com.example.SignalService";
constexpr const char* kObjectPath = "/com/example/SignalService";
constexpr const char* kInterfaceName = "com.example.SignalInterface";
constexpr const char* kSignalName = "Tick";

std::atomic<bool> running{true};

void handleSignal(int) {
    running.store(false);
}
}

int main(int argc, char* argv[]) {
    unsigned int periodMs = 1000;
    if (argc > 1) {
        try {
            periodMs = static_cast<unsigned int>(std::stoul(argv[1]));
            if (periodMs == 0U) {
                std::cerr << "Period must be greater than 0 ms" << std::endl;
                return 1;
            }
        } catch (...) {
            std::cerr << "Invalid period. Usage: ./signal_service [period_ms]" << std::endl;
            return 1;
        }
    }

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    DBusConn dbusConn(DBUS_BUS_SESSION);
    DBusConnection* conn = dbusConn.getConn();
    if (conn == nullptr) {
        return 1;
    }

    DBusError error;
    dbus_error_init(&error);

    int requestResult = dbus_bus_request_name(conn, kServiceName, DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Name Error: " << error.message << std::endl;
        dbus_error_free(&error);
        return 1;
    }
    if (requestResult != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        std::cerr << "Unable to own service name: " << kServiceName << std::endl;
        return 1;
    }

    std::cout << "Signal service started" << std::endl;
    std::cout << "Service: " << kServiceName << std::endl;
    std::cout << "Object Path: " << kObjectPath << std::endl;
    std::cout << "Interface: " << kInterfaceName << std::endl;
    std::cout << "Signal: " << kSignalName << std::endl;
    std::cout << "Period: " << periodMs << " ms" << std::endl;

    uint32_t counter = 0;
    while (running.load()) {
        ++counter;

        DBusMessage* signalMessage = dbus_message_new_signal(kObjectPath, kInterfaceName, kSignalName);
        if (signalMessage == nullptr) {
            std::cerr << "Failed to allocate signal message" << std::endl;
            break;
        }

        const char* text = "periodic tick";
        if (!dbus_message_append_args(signalMessage,
                                      DBUS_TYPE_UINT32,
                                      &counter,
                                      DBUS_TYPE_STRING,
                                      &text,
                                      DBUS_TYPE_INVALID)) {
            std::cerr << "Failed to append signal arguments" << std::endl;
            dbus_message_unref(signalMessage);
            break;
        }

        if (!dbus_connection_send(conn, signalMessage, nullptr)) {
            std::cerr << "Failed to send signal" << std::endl;
            dbus_message_unref(signalMessage);
            break;
        }
        dbus_connection_flush(conn);
        dbus_message_unref(signalMessage);

        std::cout << "[EMIT] Tick counter=" << counter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(periodMs));
    }

    std::cout << "Signal service stopped" << std::endl;
    dbus_error_free(&error);
    return 0;
}
