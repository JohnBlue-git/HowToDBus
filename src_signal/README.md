# DBus Periodic Signal Example

This example demonstrates how to emit and listen to D-Bus signals periodically.

## Structure

- `signal_service.cpp`: emits `Tick` signal every period
- `signal_client.cpp`: subscribes to `Tick` signal and prints payload
- `CMakeLists.txt`: build configuration

## Signal Definition

- Service name: `com.example.SignalService`
- Object path: `/com/example/SignalService`
- Interface: `com.example.SignalInterface`
- Signal: `Tick(uint32 counter, string text)`

## Build

```bash
cd /workspaces/HowToDBus/src_signal
mkdir -p build
cd build
cmake ..
make
```

## Run (same D-Bus session)

```bash
cd /workspaces/HowToDBus/src_signal
dbus-run-session -- bash -c './build/signal_service 500 > /tmp/signal_service.log 2>&1 & SERVICE_PID=$!; sleep 1; timeout 3 ./build/signal_client; CLIENT_EC=$?; kill $SERVICE_PID; wait $SERVICE_PID 2>/dev/null; echo CLIENT_EC=$CLIENT_EC; echo ---SERVICE_LOG---; cat /tmp/signal_service.log'
```

## Separate terminals

If you use separate terminals, both processes must use the same `DBUS_SESSION_BUS_ADDRESS`.

Terminal 1 (start session bus and service):

```bash
cd /workspaces/HowToDBus/src_signal
dbus-run-session -- bash -lc 'echo "$DBUS_SESSION_BUS_ADDRESS" > /tmp/signal_dbus_addr; ./build/signal_service 1000'
```

Terminal 2 (reuse the same session bus address):

```bash
cd /workspaces/HowToDBus/src_signal
export DBUS_SESSION_BUS_ADDRESS="$(cat /tmp/signal_dbus_addr)"
./build/signal_client
```

## Notes

- Default period is 1000 ms if no argument is provided.
- Both programs use `DBUS_BUS_SESSION`.
- If `DBUS_SESSION_BUS_ADDRESS` is not set in headless environments, you may see:
	`org.freedesktop.DBus.Error.NotSupported` and `Unable to autolaunch a dbus-daemon without a $DISPLAY for X11`.
- Stop with `Ctrl+C`.
