cmake_minimum_required(VERSION 3.10)

# Project
project(SampleSignal VERSION 1.0)

# Pthread flag
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find and link against the dbus-1 library
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
include_directories(
    ${DBUS_INCLUDE_DIRS}
)
link_libraries(
    ${DBUS_LIBRARIES}
)

# Include header
include_directories(signal_service PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Executable
add_executable(signal_service ${CMAKE_CURRENT_SOURCE_DIR}/signal_service.cpp)
add_executable(signal_client ${CMAKE_CURRENT_SOURCE_DIR}/signal_client.cpp)

# Additional flags
add_compile_options(
  -Wall -Wextra
)

# Find Thread (could be pthread or win-thread)
find_package(Threads REQUIRED)
target_link_libraries(signal_service
    Threads::Threads
)
