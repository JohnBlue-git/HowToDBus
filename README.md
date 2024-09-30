
## What is DBus

D-Bus is an integral part of the Linux operating system and in true Linux fashion is heavily used and poorly documented. After hours of scouring the internet, I have pulled together some resources that start to paint a more complete picture of D-Bus, the problem D-Bus solves and how to interact with it programatically (in C/C++). \

D-Bus at its essence is a message passing system that facilitates interprocess communication and generically abstracts a service's functionality. This allows a service to provide functionality and information in such a way that it can be accessed across process boundaries, language boundaries, licensing restrictions and even network boundaries! \

Reference:
- https://dbus.freedesktop.org/doc/dbus-tutorial.html#:~:text=D-Bus%20is%20a%20system%20for%20interprocess
- https://dbus.freedesktop.org/doc/dbus-specification.html#:~:text=The%20bus%20itself%20owns%20a%20special
- https://dbus.freedesktop.org/doc/dbus-api-design.html#:~:text=A%20D-Bus%20API%20is%20a%20specification

(under construction ...)

## System VS Session

### System D-Bus:
- System Services Communication
    - Used for communication between system services, such as hardware drivers, system daemons, and other background services.
    - Example: NetworkManager communicating with system services to manage network connections.
- Root Privilege Operations
    - Handles operations that require elevated privileges, allowing non-privileged applications to request actions from privileged services.
    - Example: Managing system resources like power management or hardware access.
- System State Notifications
    - Facilitates notifications about system events (e.g., battery status changes, hardware status).
    - Example: A service that notifies applications when a device is connected or disconnected.
- Inter-Service Communication
    - Enables different system services to communicate and coordinate actions, improving system efficiency and responsiveness.
    - Example: A logging service that collects logs from various daemons.

### Session D-Bus:
- User Session Applications
    - Primarily used for communication between applications running in the same user session.
    - Example: A media player communicating with a desktop environment to update play/pause status.
- Desktop Environment Integration
    - Facilitates features like application launchers, notifications, and menu updates within a graphical user interface.
    - Example: A notification daemon that displays alerts from various applications.
- User-Specific Configuration
    - Allows applications to retrieve or set user-specific preferences and configurations.
    - Example: A settings application that updates user preferences for a particular app.
- Dynamic Service Discovery
    - Enables applications to discover services offered by other applications in the same user session dynamically.
    - Example: A file manager that discovers and interacts with a running text editor.

### Session D-Bus benefits
- Isolation of Services
    - A D-Bus session creates an isolated environment for communication between applications. Each user session can have its own D-Bus session, which helps prevent interference between services running for different users.
- Access Control
    - D-Bus implements a permission model where services can restrict access based on the session. By starting a new session, you ensure that only processes within that session can communicate with each other, enhancing security.
- User-Specific Services
    - Many applications (especially desktop applications) register services on the session bus that are relevant only to the user currently logged in. Starting a D-Bus session ensures that you can interact with these user-specific services.
- Environment Setup
    - A D-Bus session initializes necessary environment variables (like DBUS_SESSION_BUS_ADDRESS) that applications rely on to connect to the session bus. Without this, applications may fail to find and connect to the bus.
- Development and Testing
    - For developers, starting a D-Bus session allows for testing applications in a controlled environment, simulating how they would operate in a full user session.
- Access to Introspection and Signals
    - When a D-Bus session is active, you can leverage features like introspection (discovering available interfaces and methods) and signals (event notifications) effectively.

### How to dbus session

Run dbus-launch / dbus-run-session
```console
# pre-requirement
ps -ef | grep -i dbus-daemon

# run a terminal within a new D-Bus session
dbus-run-session -- gnome-terminal

# create and interact with D-Bus commands:
dbus-run-session -- bash

# check session
echo $DBUS_SESSION_BUS_ADDRESS

# create different sessions
dbus-run-session -- ./hello_service.sh &
dbus-run-session -- ./hello_service.sh &
```

Start a session and save the address
```console
dbus-run-session -- bash -c 'echo $DBUS_SESSION_BUS_ADDRESS > /tmp/session_address.txt && sleep 10'
```

Use the address in the Program
```C++
#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>

int main() {
    DBusError error;
    DBusConnection *connection;
    
    // Initialize error
    dbus_error_init(&error);

    // Read the session bus address from the file
    FILE *file = fopen("/tmp/session_address.txt", "r");
    char address[256];
    if (file) {
        fgets(address, sizeof(address), file);
        fclose(file);
    } else {
        fprintf(stderr, "Could not read session address\n");
        return 1;
    }

    // Set the environment variable
    // !!! Specific the address for the application to connect
    // !!! Different applications can set different DBUS_SESSION_BUS_ADDRESS values simultaneously to connect to specific D-Bus sessions.
    setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
    // or
    //setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/tmp/session_address", 1);

    // Now get the session bus connection
    connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

    // ...

    return 0;
}
```

## Commands related to DBus

### Configuration files
Location
```console
# for system services
/etc/dbus-1/system.d/

# for session services
/etc/dbus-1/session.d/
```
Example
```xml
<?xml version="1.0" encoding="UTF-8"?> <!-- -*- XML -*- -->

<!DOCTYPE busconfig PUBLIC
 "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>

  <!-- This configuration file specifies the required security policies
       for the PackageKit to work. -->

  <!-- Only user root can own the PackageKit service -->
  <policy user="root">
    <allow own="org.freedesktop.PackageKit"/>
  </policy>

 <!-- Allow anyone to call into the service - we'll reject callers using PolicyKit -->
  <policy context="default">
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.PackageKit"/>
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.PackageKit.Transaction"/>
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.PackageKit.Offline"/>
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.DBus.Properties"/>
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.DBus.Introspectable"/>
    <allow send_destination="org.freedesktop.PackageKit"
           send_interface="org.freedesktop.DBus.Peer"/>
  </policy>

</busconfig>
```

### Run dbus daemon
There are a few way to run the dbus daemon: \
systemctl
```console
sudo systemctl restart dbus
```
service
```console
# usage: service < option > | --status-all | [ service_name [ command | --full-restart ] ]
service

# check status
service --status-all
 [ ? ]  binfmt-support
 [ - ]  dbus
 [ ? ]  hwclock.sh
 [ - ]  procps
 [ - ]  rsync
 [ + ]  ssh
 [ - ]  x11-common

# run dbus
sudo service dbus --full-restart
```

### busctl
Command
```console
# search service
busctl | grep -i <service name>

# tree service -> object path
busctl tree <service name>

# introspect object -> interface
busctl introspect <service name> <object path>

# method call
busctl call --system <service name> <object path> <interface name> <method> [arguments ...]

# get property
busctl get-property --system <service name> <object path> <interface name> <property name> [arguments ...]

# specefic session
busctl --user [commands ...]
# or
DBUS_SESSION_BUS_ADDRESS=unix:path=/tmp/session_address.txt busctl --session [commands ...]
```

Ref: https://manpages.ubuntu.com/manpages/focal/zh_TW/man1/busctl.1.html

## gdbus
Command
```console
gdbus call --system --dest=org.freedesktop.DBus --object-path /org/freedesktop/DBus --method org.freedesktop.DBus.Introspectable.Introspect

gdbus call --system --dest=<service name> --object-path <object path> --method <interface name>.<method name> [arguments ...]
```

## dbus-send
Command
```console
dbus-send --system --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.Introspectable.Introspect

dbus-send --system --print-reply --dest=<service name> --object-path <object path> --method <interface name>.<method name> [arguments ...]
```

## How to code with DBus

### Check header files exit

Command for checking
```console
# use apt-file (but not all linux support) 
sudo apt install apt-file
sudo apt-file search dbus/dbus.h

# use find
sudo find /usr -name dbus.h
```

Common header files locations:
- /usr/include/dbus-1.0/dbus/dbus.h
- /usr/local/include/dbus-1.0/dbus/dbus.h

### What if header files / ... are not installed 

Install library
```console
sudo apt-get install libdbus-1-dev
```

Manually download source code and build in local
```console
# download source (we can replace 1.12.20 with the latest version available)
wget https://dbus.freedesktop.org/releases/dbus/dbus-1.12.20.tar.gz

# untar tarball
tar -xzf dbus-1.12.20.tar.gz

# go to directory
cd dbus-1.12.20

# configure the Build:
./configure

# compile the Source:
make

# install the Library:
sudo make install

# remove unuused source code and tar ball
...
```

### C++ wrapped sample codes

Location:
- **dbus connection**:
    - include
- **sample dbus client**:
    - src_sample_client
- **self defined dbus service**
    - src_hello

Common dbus functions:
```C++
// DBus error object
DBusError dbus_error;
dbus_error_init(&dbus_error);


// Connect to D-Bus
DBusConnection* dbus_conn = nullptr;
dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error)
dbus_connection_unref(dbus_conn);


// Compose remote procedure call (client side)
DBusMessage* dbus_method_call = nullptr;
dbus_method_call = dbus_message_new_method_call(<service name>, <object path>, <interface name>, <method name>)
dbus_connection_unref(dbus_method_call);

// Append arguments to remote procedure call (client side)
int arg = 42;
dbus_message_append_args(dbus_method_call, DBUS_TYPE_INT32, &arg, DBUS_TYPE_INVALID);

// Invoke remote procedure call, block for response (client side)
DBusMessage* dbus_reply = nullptr;
dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_method_call, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
dbus_connection_unref(reply);

// Parse response (client side)
const char* dbus_result = nullptr;
dbus_message_get_args(dbus_reply, &dbus_error, DBUS_TYPE_STRING, &dbus_result, DBUS_TYPE_INVALID)

// Request a name on the bus (server side)
dbus_bus_request_name(dbus_conn, <service name>, DBUS_NAME_FLAG_REPLACE_EXISTING, &dbuus_error);

// Wait for connection and requeust (server side)
dbus_connection_read_write(dbus_conn, 1000);
// or
dbus_connection_read_write_dispatch(dbus_conn, -1);

// Pop request message (server side)
DBusMessage* message;
message = dbus_connection_pop_message(connection);
dbus_connection_unref(message);

// Check request (server side)
dbus_message_is_method_call(message, <interface name>,<method name>)) {


// Compose reply message (server side)
DBusMessage* reply;
reply = dbus_message_new_method_return(message);
dbus_connection_unref(reply);

// Send reply (server side)
dbus_connection_send(dbus_conn, reply, nullptr);
```

## Futer Plan

### introduction
- (not yet)

### sample client
- method: s
- method: a{sv}
- get property (not yet)
- signal (?)

### hello
- service
    - method s
    - property (?)
    - signal (?)
    - async (not yet)
- client
    - method
    - property (?)
    - signal (?)
