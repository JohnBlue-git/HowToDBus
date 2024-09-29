
## To allow self-defined service (optional)
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
```



