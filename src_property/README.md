# DBus Property Example

This example demonstrates how to implement D-Bus properties (Get/Set operations).

## Structure

- **property_service.cpp**: Service that exposes properties via D-Bus Properties interface
- **property_client.cpp**: Client that gets and sets properties
- **CMakeLists.txt**: Build configuration

## D-Bus Properties Interface

This example uses the standard `org.freedesktop.DBus.Properties` interface which provides:
- `Get(interface_name, property_name) -> variant`: Get a property value
- `Set(interface_name, property_name, value) -> void`: Set a property value
- `GetAll(interface_name) -> array of (string, variant)`: Get all properties

## Building

```bash
cd /workspaces/HowToDBus/src_property
mkdir -p build
cd build
cmake ..
make
```

## Running

### Important: D-Bus Session Requirement

Since this service uses the session D-Bus, you need to run it within a D-Bus session environment using `dbus-run-session`.

### Option 1: Run Service and Client in Same Session (Recommended for Testing)

```bash
cd /workspaces/HowToDBus/src_property
dbus-run-session -- bash -c 'timeout 10 ./build/property_service > /tmp/prop_service.log 2>&1 & sleep 2; ./build/property_client; wait'
```

### Option 2: Run Service in Terminal 1

```bash
cd /workspaces/HowToDBus/src_property
dbus-run-session -- ./build/property_service
```

### Option 3: Run Client in Terminal 2

```bash
cd /workspaces/HowToDBus/src_property
dbus-run-session -- ./build/property_client
```

**Note:** Both service and client must be run within the same `dbus-run-session` environment. If you use Option 2 and 3, make sure both terminals are within the same D-Bus session started with `dbus-run-session`.

### Why dbus-run-session?

Without `dbus-run-session`, you may encounter:
```
Connection Error: Unable to autolaunch a dbus-daemon without a $DISPLAY for X11
```

This error occurs because:
1. The system doesn't have an active X11 session
2. D-Bus cannot automatically start a session daemon
3. Using `dbus-run-session` creates an isolated D-Bus session for your application

## Properties

The service provides the following properties:

### Integer Properties
- `Temperature` (default: 25)
- `Brightness` (default: 80)

### String Properties
- `DeviceName` (default: "PropertyDevice")
- `Status` (default: "Ready")

## Example Output

Service output:
```
Property Service started...
Service: com.example.PropertyService
Object Path: /com/example/PropertyService
Interface: org.freedesktop.DBus.Properties

Property set: Temperature = 30
Integer Properties: Temperature=30 Brightness=100
String Properties: DeviceName=UpdatedDevice Status=Running
```

Client output:
```
=== Property Client ===

--- Getting Initial Properties ---
[GET] Temperature = 25
[GET] Brightness = 80
[GET] DeviceName = PropertyDevice
[GET] Status = Ready

--- Setting New Properties ---
[SET] Temperature = 30 (success)
[SET] Brightness = 100 (success)
[SET] DeviceName = UpdatedDevice (success)
[SET] Status = Running (success)

--- Getting Updated Properties ---
[GET] Temperature = 30
[GET] Brightness = 100
[GET] DeviceName = UpdatedDevice
[GET] Status = Running
```

## To allow self-defined service

Configuration Location:
- for system services: `/etc/dbus-1/system.d/`
- for session services: `/etc/dbus-1/session.d/`

Create `com.example.PropertyService.conf`:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <!-- Allow this process to own the service name -->
  <policy user="<current user>">
    <allow own="com.example.PropertyService"/>
  </policy>

  <!-- Allow clients to call methods on this service/interface -->
  <policy context="default">
    <allow send_destination="com.example.PropertyService"/>
    <allow send_interface="org.freedesktop.DBus.Properties"/>
  </policy>
</busconfig>
```

### For System DBus
1. Copy the configuration file to system directory:
   ```bash
   sudo cp com.example.PropertyService.conf /etc/dbus-1/system.d/
   ```

2. Restart the D-Bus daemon:
   ```bash
   sudo systemctl restart dbus
   ```

3. Modify `property_service.cpp` to use `DBUS_BUS_SYSTEM` instead of `DBUS_BUS_SESSION`

4. Run the service with sudo:
   ```bash
   sudo ./build/property_service
   ```

### For Session DBus
1. Copy the configuration file to session directory:
   ```bash
   cp com.example.PropertyService.conf ~/.local/share/dbus-1/services/
   ```
   (Note: You may need to create the directory if it doesn't exist)

## Notes

- This example uses the D-Bus session bus (DBUS_BUS_SESSION)
- The service uses the standard `org.freedesktop.DBus.Properties` interface for property access
- Both integer (int32) and string (string) property types are supported
- The example demonstrates a simple property storage mechanism using C++ maps
