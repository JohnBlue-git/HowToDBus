# Advanced DBus Sample (Calculator Service)

This sample demonstrates DBus service and client with more complex argument handling.

## Service Definition

**Service Name:** `com.example.CalcService`

**Object Path:** `/com/example/CalcService`

**Interface:** `com.example.CalcInterface`

### Methods

#### 1. Add(int32, int32) → int32
```
Add(a: int32, b: int32) -> (result: int32)
```
- **Arguments:** Two 32-bit integers
- **Returns:** Sum as 32-bit integer
- **Example:** Add(5, 3) → 8

#### 2. Multiply(double, double) → double
```
Multiply(a: double, b: double) -> (result: double)
```
- **Arguments:** Two double-precision floats
- **Returns:** Product as double
- **Example:** Multiply(2.5, 4.0) → 10.0

#### 3. Concatenate(string, string) → string
```
Concatenate(s1: string, s2: string) -> (result: string)
```
- **Arguments:** Two strings
- **Returns:** Concatenated string with " + " separator
- **Example:** Concatenate("Hello", "World") → "Hello + World"

#### 4. ProcessData(string, int32, double) → string
```
ProcessData(name: string, age: int32, salary: double) -> (message: string)
```
- **Arguments:** Employee name (string), age (int32), salary (double)
- **Returns:** Formatted employee information string
- **Example:** ProcessData("John", 30, 50000.50) → "Employee: John, Age: 30, Salary: $50000.500000"

## Implementation Reference

### Service Implementation (sample_service.cpp)

**Key Components:**

1. **Controller Class** (`CalculatorController : public IRouter`)
   - Implements `handleRequest()` to route incoming method calls
   - Each method has dedicated handler: `prepareReplyAdd()`, `prepareReplyMultiply()`, etc.
   - Uses `dbus_message_is_method_call()` to identify which method was invoked
   - Uses `dbus_message_get_args()` to extract arguments from the message
   - Uses `dbus_message_new_method_return()` and `dbus_message_append_args()` to send response

2. **Service Class** (`BlockAcceptService : public DBusServer`)
   - Inherits from DBusServer wrapper
   - Implements blocking accept pattern (processes one message at a time)
   - Main loop: read → pop message → handle → clean up

### Client Implementation (sample_client.cpp)

**Key Components:**

1. **Client Class** (`CalculatorClient : public DBusClient`)
   - Wrapper around DBusClient base class
   - One method for each service method: `callAdd()`, `callMultiply()`, etc.
   - Each `call*()` method:
     - Takes input arguments
     - Provides callback function for response handling
     - Uses `dbus_message_append_args()` to build the call
     - Uses `parse*()` method to extract response

2. **Parse Methods**
   - Extract return values from reply message
   - Invoke callback with parsed result
   - Handle errors gracefully

### Build Configuration (CMakeLists.txt)

Key differences from hello example:
- Creates both `sample_service` and `sample_client` executables
- Links against `dbus-1` library via pkgconfig
- Includes pthread support for blocking operations
- Adds compiler warnings `-Wall -Wextra`

## Building

```bash
cd /workspaces/HowToDBus/src_sample_advanced
mkdir -p build
cd build
cmake ..
make
```

## Running

### Terminal 1 - Start Service
```bash
cd /workspaces/HowToDBus/src_sample_advanced/build
sudo ./sample_service
```

### Terminal 2 - Run Client
```bash
cd /workspaces/HowToDBus/src_sample_advanced/build
sudo ./sample_client
```

## Argument Types Reference

DBus type signatures used:
- `DBUS_TYPE_INT32` - 32-bit signed integer
- `DBUS_TYPE_DOUBLE` - IEEE 754 double precision float
- `DBUS_TYPE_STRING` - UTF-8 string
- `DBUS_TYPE_INVALID` - Message terminator (required)

## Pattern Comparison with hello_service.cpp

### Similarities
- Same controller + service architecture
- Same message routing pattern with `dbus_message_is_method_call()`
- Same BlockAcceptService blocking implementation
- Same DBus connection wrapper usage

### Differences
| Aspect | hello_service | sample_service |
|--------|---------------|-----------------|
| Methods | 1 (Hello) | 4 (Add, Multiply, Concatenate, ProcessData) |
| Arg Types | String only | Mixed: int32, double, string |
| Return | Single string | Varies by method |
| Complexity | Simple single-arg | Multiple args of different types |
| Use Case | Demo/example | Production-like real-world scenario |

## To allow self-defined service
Configuration Location:
- for system services: /etc/dbus-1/system.d/
- for session services: /etc/dbus-1/session.d/
Create com.example.CalcService.conf:
```xml
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
   <!-- Allow this process to own the service name -->
   <policy user="<current user>">
      <allow own="com.example.CalcService"/>
   </policy>

   <!-- Allow clients to call methods on this service/interface -->
   <policy context="default">
      <allow send_destination="com.example.CalcService"/>
      <allow send_interface="com.example.CalcInterface"/>
    </policy>
</busconfig>
```
