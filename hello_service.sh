#!/bin/bash

dbus-run-session -- bash -c 'echo "Session Address: $DBUS_SESSION_BUS_ADDRESS" && sleep 10'
