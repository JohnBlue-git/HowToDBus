#pragma once

#include <iostream>
#include <dbus/dbus.h>

//
// Connection
//

class DBusConn {
private:
    DBusError error;
private:
    DBusConnection* conn;

public:
    // Constructor
    DBusConn(DBusBusType type)
    {
        dbus_error_init(&error);

        // Connect to D-Bus
        if ( nullptr == (conn = dbus_bus_get(type, &error)) ) {
            std::cerr << error.name << std::endl << error.message << std::endl;
        }
    }
    // delete
    DBusConn(DBusConn&& other) = delete;
    DBusConn& operator=(DBusConn&& other) = delete;
    DBusConn(const DBusConn&) = delete;
    DBusConn& operator=(const DBusConn&) = delete;
    // De-Constructor
    ~DBusConn() {
        dbus_error_free(&error);

        // Applications must not close shared connections -
        // see dbus_connection_close() docs. This is a bug in the application.
        // dbus_connection_close(dbus_conn);

        // When using the System Bus, unreference the connection instead of closing it
        if (conn) {
            dbus_connection_unref(conn);
        }
    }

public:
    DBusConnection* getConn() {
        return conn;
    };
};