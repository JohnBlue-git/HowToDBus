cmake_minimum_required(VERSION 3.10)

# Project
project(SampleClient VERSION 1.0)

# C++ flag
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include header
include_directories(sample_client PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )
# Source code
file(GLOB SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  )
# Executable
add_executable(sample_client ${SOURCES})
# Additional flags
target_compile_options(sample_client PRIVATE
  -Wall -Wextra
  )

# Find and link against the dbus-1 library
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
target_include_directories(sample_client PRIVATE
	${DBUS_INCLUDE_DIRS}
	)
target_link_libraries(sample_client PRIVATE
	${DBUS_LIBRARIES}
	)

#g++ sample_client.cpp -o sample_client $(pkg-config dbus-1 --cflags) -ldbus-1 -Wall -Wextra

#rm -rf * && cmake .. && make

#sudo service dbus --full-restart
#sudo ./sample_client
