
## To allow  user level permission for specific service (optional)
Configuration Location:
- for system services: /etc/dbus-1/system.d/
- for session services: /etc/dbus-1/session.d/
Modify org.freedesktop.PackageKit.conf:
```xml
<policy user="your-username">
    <allow send_destination="org.freedesktop.DBus" send_interface="org.freedesktop.DBus.Debug.Stats" />
</policy>
```

## Raw command build
```console
# cd build
mkdir build && cd build

# client
g++ ../sample_client.cpp -o sample_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra
```

## CMake build
```console
# cd build
mkdir build && cd build

# cmake
rm -rf * && cmake .. && make
```

## Run
```console
# start dbus daemon
sudo service dbus --full-restart

# run client
sudo ./sample_client

# raw dbus client command
sudo busctl tree --system org.freedesktop.DBus
sudo busctl introspect --system org.freedesktop.DBus /org/freedesktop/DBus
#
sudo busctl call --system org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.Debug.Stats GetStats
sudo busctl call --system org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.Introspectable Introspect
#
sudo dbus-send --system --print-reply \
    --dest=org.freedesktop.DBus \
    /org/freedesktop/DBus \
    org.freedesktop.DBus.Debug.Stats.GetStats
sudo dbus-send --system --print-reply \
    --dest=org.freedesktop.DBus \
    /org/freedesktop/DBus \
    org.freedesktop.DBus.Introspectable.Introspect


# property command
   Name                               Kind      Signature
org.freedesktop.DBus.Properties       interface
.Get                                  method    ss          v
.GetAll                               method    s           a{sv}
.Set                                  method    ssv
.PropertiesChanged                    signal    sa{sv}      as
#
sudo busctl call --system org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.Properties Get ss org.freedesktop.DBus Features
```

## Caution about get property
Take example:
```console
busctl get-property
  xyz.openbmc_project.State.Host \
  /xyz/openbmc_project/state/host0 \
  xyz.openbmc_project.State.Host \
  CurrentHostState

# or

busctl call
   xyz.openbmc_project.State.Host \
   /xyz/openbmc_project/state/host0 \
   org.freedesktop.DBus.Properties \
   Get ss \
   xyz.openbmc_project.State.Host CurrentHostState
```
The response type should be "variant" not "string", and the code should look like:
```C
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
        DBusError dbus_error;
    DBusConnection * dbus_conn = NULL;
    DBusMessage * dbus_msg = NULL;
    DBusMessage * dbus_reply = NULL;
    DBusMessageIter variant;
    const char* dbus_result = NULL;

    const char * service = "xyz.openbmc_project.State.Host";
    const char * path = "/xyz/openbmc_project/state/host0";
    const char * interface = "org.freedesktop.DBus.Properties";
    const char * method = "Get";
    const char * arg1 = "xyz.openbmc_project.State.Host";
    const char * arg2 = "CurrentHostState";

    // Initialize D-Bus error
    dbus_error_init(&dbus_error);

    // Connect to D-Bus
    if ( NULL == (dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error)) ) {
        printf("%s:%s\n", dbus_error.name, dbus_error.message);

    // Compose remote procedure call
    } else if ( NULL == (dbus_msg = dbus_message_new_method_call(service, path, interface, method)) ) {
        printf("ERROR: ::dbus_message_new_method_call - Unable to allocate memory for the message!");

    // Append arguments
    } else if ( false == dbus_message_append_args(dbus_msg,
                                                DBUS_TYPE_STRING, &arg1,
                                                DBUS_TYPE_STRING, &arg2,
                                                DBUS_TYPE_INVALID) ) {
        printf("ERROR: dbus_message_append_args - Unable to append argument!");

    // Invoke remote procedure call, block for response
    } else if ( NULL == (dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg,
                                                                                DBUS_TIMEOUT_USE_DEFAULT, &dbus_error)) ) {
        printf("%s: %s\n", dbus_error.name, dbus_error.message);

    // Extract variant from reply
    } else if ( false == dbus_message_iter_init(dbus_reply, &variant) ) {
        printf("%s: %s\n", dbus_error.name, dbus_error.message);

    // Work with the results of the remote procedure call
    } else {
        DBusMessageIter sub_iter;
        dbus_message_iter_recurse(&variant, &sub_iter);
        dbus_message_iter_get_basic(&sub_iter, &dbus_result);
        printf("Connected to D-Bus as %s\n", dbus_bus_get_unique_name(dbus_conn));
        printf("Result: %s\n\n",  dbus_result);
    }

    if (dbus_reply) {
        dbus_message_unref(dbus_reply);
    }
    if (dbus_msg) {
        dbus_message_unref(dbus_msg);
    }
    if (dbus_conn) {
        dbus_connection_unref(dbus_conn);
    }

    return 0;
}
```



