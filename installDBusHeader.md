
## header

dbus/dbus.h: No such file or directory

usually in /usr/include/
but not always

## apt-get install

sudo apt-get install libdbus-1-dev

but
Reading package lists... Done
Building dependency tree       
Reading state information... Done
E: Unable to locate package libdbus-1-dev

## alternative

you can manually download and install D-Bus from source if the package is not available via your package manager. Here’s how to do it:

Downloading and Installing D-Bus from Source
wget https://dbus.freedesktop.org/releases/dbus/dbus-1.12.20.tar.gz
Replace 1.12.20 with the latest version available.

Extract the Tarball:
tar -xzf dbus-1.12.20.tar.gz
cd dbus-1.12.20

Install Build Dependencies: Before building, make sure you have the necessary tools and libraries:
sudo apt-get install build-essential autoconf automake libtool
You might also need other dependencies like libglib2.0-dev.

Configure the Build:
./configure

Compile the Source:
make

Install the Library:
sudo make install
