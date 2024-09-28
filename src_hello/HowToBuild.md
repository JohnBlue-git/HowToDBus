
Configuration Location:

for system services
/etc/dbus-1/system.d/
for session services
/etc/dbus-1/session.d/


com.example.HelloService.conf

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





sudo service dbus --full-restart




# <thread>, <future>, async related, need -lpthread
g++ ../hello_service.cpp -o hello_service $(pkg-config dbus-1 --cflags) -ldbus-1 -lpthread -Wall -Wextra -DBLOCK_ACCEPT

-DBLOCK_ACCEPT

./hello_service
sudo ./hello_service will trigger many thing that are not allowed and show error message

busctl call --system com.example.HelloService /com/example/HelloService com.example.HelloInterface Hello s world




g++ ../hello_client.cpp -o hello_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

./hello_client


cd build
rm -rf * && cmake .. && make
