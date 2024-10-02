
## To allow self-defined service
Configuration Location:
- for system services: /etc/dbus-1/system.d/
- for session services: /etc/dbus-1/session.d/
Create com.example.HelloService.conf:
```xml
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
    <!--   *   Allow any user to talk to our service -->
    <!-- $USER Replace with your username -->
    <policy user="*">
        <allow own="com.example.HelloService"/>
        <allow send_destination="com.example.HelloService"/>
        <allow receive_sender="com.example.HelloService"/>
        <allow send_interface="com.example.HelloInterface"/>
    </policy>
</busconfig>
```

## Raw command build
service
```console
# notes:
# std::thread, std::future, async related ... all need -lpthread

# cd build
mkdir build && cd build

# default
g++ ../hello_service.cpp -o hello_service $(pkg-config dbus-1 --cflags) -ldbus-1 -lpthread -Wall -Wextra

# service type: <BLOCK_ACCEPT | ASYNC_ACCEPT | THREAD_ACCEPT>
g++ ../hello_service.cpp -o hello_service $(pkg-config dbus-1 --cflags) -ldbus-1 -lpthread -Wall -Wextra -DASYNC_ACCEPT
```
client
```console
# cd build
mkdir build && cd build

# client
g++ ../hello_client.cpp -o hello_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra
```

## CMake build
```console
# cd build
mkdir build && cd build

# default
rm -rf * && cmake .. && make

# SERVICE_TYPE:STRING=<BLOCK_ACCEPT | ASYNC_ACCEPT | TREAD_ACCEPT>
rm -rf * && cmake .. -D SERVICE_TYPE:STRING=ASYNC_ACCEPT && make
```

## Run
```console
# start dbus daemon
sudo service dbus --full-restart

# run service
./hello_service

# run client
./hello_client

# raw dbus client command
busctl call --system com.example.HelloService /com/example/HelloService com.example.HelloInterface Hello s world

# note: sudo will trigger unneccessary error message
# sudo ./hello_service
# sudo ./hello_client
```
