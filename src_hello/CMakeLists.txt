cmake_minimum_required(VERSION 3.10)

# Project
project(Hello VERSION 1.0)

# C++ flag
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
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
include_directories(hello_client PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )
# Executable
add_executable(hello_client ${CMAKE_CURRENT_SOURCE_DIR}/hello_client.cpp)
add_executable(hello_service ${CMAKE_CURRENT_SOURCE_DIR}/hello_service.cpp)
# Additional flags
add_compile_options(
  -Wall -Wextra
  )
# Additional definitions
set(SERVICE_TYPE "BLOCK_ACCEPT" CACHE STRING "")
if(SERVICE_TYPE STREQUAL "BLOCK_ACCEPT")
    target_compile_definitions(hello_service PRIVATE BLOCK_ACCEPT)
elseif(SERVICE_TYPE STREQUAL "ASYNC_ACCEPT")
    target_compile_definitions(hello_service PRIVATE ASYNC_ACCEPT)
elseif(SERVICE_TYPE STREQUAL "THREAD_ACCEPT")
    target_compile_definitions(hello_service PRIVATE THREAD_ACCEPT)
else()
    target_compile_definitions(hello_service PRIVATE BLOCK_ACCEPT)
endif()

# Find Thread (could be pthread or win-thread)
find_package(Threads REQUIRED)
target_link_libraries(hello_service
    Threads::Threads
    )
