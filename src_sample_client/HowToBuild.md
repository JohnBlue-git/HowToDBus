
sudo service dbus --full-restart




g++ ../sample_client.cpp -o sample_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

sudo ./sample_client




rm -rf * && cmake .. && make
