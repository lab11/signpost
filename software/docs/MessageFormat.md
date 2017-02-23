Message Format
==============


Link
----

Signpost currently uses the well-known I<sup>2</sup>C bus as its link layer

```text
        0                   1                   2
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Destination   |W|A| Payload Byte 0  |A| ...
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### Destination

The address of this message. Module addresses SHOULD be discovered at runtime
during module initialization. Some well-known addresses are predefined:

  - Controller: 0x20
  - Stroage: 0x21
  - Radio: 0x22


Net
---

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |Version| Flags | Source        |0| Sequence Number             |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Length                          | Offset                      |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### Version

The protocol version in use. Currently, the only valid version is version 1.

### Flags

  - Bit 4: Reserved
  - Bit 5: Reserved
  - Bit 6: IsEncrypted - 1 if message is encrypted, 0 if clear text
  - Bit 7: IsFragment - 1 if message has more data, 0 if end of message

### Source

The 7 bit source address of the device sending this message.

### Sequence Number

A monotonically increasing message counter.

### Length

The length of the complete message in bytes.

### Offset

The offset into the complete message in bytes.


Protocol
--------

```text
                  0      7 8     15 16    23 24    31
                 +--------+--------+--------+--------+
                 |                                   |
                 |                IV                 |
                 |             (optional)            |
                 |                                   |
                 +--------+--------+--------+--------+
                 |
                 |          data octets ...
                 +---------------- ...
                 |                                   |
                 |                                   |
                 |               HMAC                |
                 |                                   |
                 |                                   |
                 |                                   |
                 |                                   |
                 |                                   |
                 +--------+--------+--------+--------+
```

### IV

Initialization vector, used only if payload is encrypted.

### HMAC

HMAC over the IV and data. This field need not be word-aligned.


Application
-----------

```text
                  0      7 8     15 16    23 24    31
                 +--------+--------+--------+--------+
                 | Frame  |  Api   |  Msg   | MsgId  |
                 | Type   |  Type  |  Type  | (opt)  |
                 +--------+--------+--------+--------+
                 |          data octets ...
```

### FrameType

Indicates the purpose of the application message

  - `0x00: Notification`
     - A one-off message that is not in response to an immediate mesage,
       nor does it require a reply.
     - This frame MAY include a `MsgId`.
  - `0x01: Command`
     - A message requesting an action to be performed.
     - A Command message MUST include the `MsgId` field, which MUST be an Id
       that is not currently used by an active Command request from this sender.
     - The recipient of a Command message MUST reply with either a `Response` or
       `Error` frame, but not both.
  - `0x02: Response`
     - A response message is generated when the recipient of a Command frame
       understands the `MessageType` and has completed the command request
          - __Note:__ A request may be considered complete if a long-running
            action has been started, but not yet completed, but the response
            indicates completion of starting the task.
     - The response MUST include a `MsgId` field and the provided Id MUST match
       the `Command` frame for this request.
     - __Note:__ A Command request that fails (i.e. could not complete the
       requested command) MUST still generate a `Response` message, not an
       `Error`.
  - `0x03: Error`
     - An error message is generated when the recipient of a Command frame
       does not know how to handle the supplied `ApiType` and/or `MessageType.
     - The error message MUST reply with the same `ApiType`, `MessageType`, and
       `MsgId` supplied in the original Command request


### ApiType

Indicates which API this message applies to. Currently the following ApiTypes are
well-defined:

  - `0x01 (InitializationApiType)`: [Initialization](#0x01-initialization)
  - `0x02 (StorageApiType)`:        [Storage](#0x02-storage)
  - `0x03 (NetworkingApiType)`:     [Networking](#0x03-networking)
  - `0x04 (ProcessingApiType)`:     [Processing](#0x04-processing)
  - `0x05 (EnergyApiType)`:         [Energy](#0x05-energy)
  - `0x06 (TimeLocationApiType)`:   [Time and Location](#0x06-time-and-location)


### MessageType

This field is used to specify which API function is currently requested. See each
API documentation for more details on the MessageTypes for each API.


### MsgId

A unique ID used to identify each Command currently in flight. The sender is
responsible for ensuring that all active IDs are unique.

A Command MUST use a new ID which shall be considered active after successful
delivery of a Command message.

An Error response MUST be treated as the final message for an ID, which shall
be considered inactive and available for reuse upon receipt of an Error message.

A Response mesage MAY be treated as the final message for an ID, however long-running
operations MAY keep the ID alive after the inital Response message, using it with
subsequent Notification messages. It is Application-defined when such IDs are
considered inactive and available for reuse.




API Types
----------

### `0x01`: Initialization

#### `signpost_initialization_module_init(uint8_t i2c_address, api_handler** api_handler)`
Initialize this module. Must be called before any other Signpost API methods.

Parameters:

`i2c_address`: Address to initialize this module with

`api_handlers`: Array of signpost APIs that this calling module implements The
final element of this array MUST be NULL.

Elements of the array are pointers to static (pointer must be valid forever) `api_handler_t` structs:
```c
typedef struct api_handler {
    signbus_api_type_t       api_type;
    signpost_api_callback_t  callback;
} api_handler_t;
```
`api_type`: One of the existing [Signpost APIs](#apitype).

`callback`: A function pointer to your handler for this API.

This array also MUST be static. Modules that implement no APIs MUST pass
`SIGNPOST_INITIALIZATION_NO_APIS` instead of a list.

### `0x02`: Storage

#### `signpost_storage_write(uint8_t* data, size_t len, Storage_Record_t* record_pointer)`
Write data to the Storage Master.

Parameters:

`data`: Data to write.

`len`: Length of data.

`record_pointer`: Pointer to record that will point to location of written data.

Sent Message:

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x02     | Type: 0x00    | Data
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         Data (cont.)           variable length                        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Response Message:

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x02     | Type: 0x00    | RecordPointer |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

`RecordPointer`: Pointer to where data was written.

### `0x03`: Networking

#### `signpost_networking_post(const char* url, http_request request, http_response* response)`
Send an HTTP POST to the radio module.

Parameters:

`url`: Destination to POST to.

`request`: Request to send. Of the form:
```c
typedef struct{
   uint8_t num_headers;
   http_header* headers;
   uint16_t body_len;
   const uint8_t* body;
} http_request;
```
`response`: Pointer to response. Of the form:
```c
typedef struct {
    uint16_t status;
    uint16_t reason_len;
    char* reason;
    uint8_t num_headers;
    http_response_header* headers;
    uint16_t body_len;
    uint8_t* body;
} http_response;
```

Sent Message:

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x03     | Type: 0x00    | urllen (LSB)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         urllen (MSB)  | url (variable length)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                   ...
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  num_headers  | header_1_len   | header_1 (variable length)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                   ...
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |  header_n_len   | header_n (variable length)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                   ...
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | bodylen (LSB)  | bodylen (MSB) | body (variable length)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                   ...                                 |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
TODO fill this out:

Response Message:
```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x03     | Type: 0x00    | URLlen (LSB)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         URLlen (MSB)  | URL (variable length)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                   ...
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```


### `0x04`: Processing
#### Note, this API is currently **not** working.

#### `signpost_processing_init(const char* path)`
Initialize, provide the path to the python module used by the RPC

parameters:

`path`: linux-style path to location of python module to handle this
            modules rpcs (e.g. `/path/to/python/module.py`)

#### `signpost_processing_oneway_send(uint8_t* buf, uint16_t len)`
Send an RPC without an expected response

parameters:
`buf`: buffer containing RPC to send

`len`: length of buf

#### `signpost_processing_twoway_send(uint8_t* buf, uint16_t len)`
Send an RPC with an expected response

parameters:
`buf`: buffer containing RPC to send

`len`: length of buf

#### `signpost_processing_twoway_receive(uint8_t* buf, uint16_t* len)`

parameters:
`buf`: buffer to fill with RPC response

`len`: length of RPC response

#### `int signpost_processing_reply(uint8_t src_addr, uint8_t message_type, uint8_t* response, uint16_t response_len)`

Reply from Storage Master to RPC requesting module

parameters:
`src_addr`: address of module that originally requested the RPC

`message_type`: type of RPC message

`response`: RPC response from compute resource

`response_len`: len of response

### `0x05`: Energy

#### `signpost_energy_query(signpost_energy_information_t* energy)`

Query the controller for energy information

parameters:

`energy`: an `energy_information_t` struct to fill. Of the form:

```
typedef struct __attribute__((packed)) energy_information {
    uint32_t    energy_limit_24h_mJ;
    uint32_t    energy_used_24h_mJ;
    uint16_t    current_limit_60s_mA;
    uint16_t    current_average_60s_mA;
    uint8_t     energy_limit_warning_threshold;
    uint8_t     energy_limit_critical_threshold;
} signpost_energy_information_t;
```

#### `signpost_energy_query_async(signpost_energy_information_t* energy, signbus_app_callback_t cb)`

parameters:

`energy`: an `energy_information_t` struct to fill

`cb`: the callback to call when energy information is collected

#### `signpost_energy_query_reply(uint8_t destination_address, signpost_energy_information_t* info)`

parameters:

`destination_address`: requesting address for this energy information

`info`: energy information

### `0x06`: Time and Location

#### `signpost_timelocation_get_time(signpost_timelocation_time_t* time)`
Get time from controller

parameters:

`time`: `signpost_timelocation_time_t` struct to fill. Of the form:

```
typedef struct __attribute__((packed)) {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint8_t  satellite_count;
} signpost_timelocation_time_t;
```


#### `signpost_timelocation_get_location(signpost_timelocation_location_t* location)`
Get location from controller

parameters:

`location`: `signpost_location_time_t` struct to fill. Of the form:

```
typedef struct __attribute__((packed)) {
    uint32_t latitude;  // Latitude in microdegrees (divide by 10^6 to get degrees)
    uint32_t longitude; // Longitude in microdegrees
    uint8_t  satellite_count;
} signpost_timelocation_location_t;
```

#### `signpost_timelocation_get_time_reply(uint8_t destination_address, signpost_timelocation_time_t* time)`

Controller reply to time requesting module

parameters:

`destination_address`: I<sup>2</sup>C address of requesting module

`time`: `signpost_timelocation_time_t` struct to return

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x06     | Type: 0x00    | Year
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         Year (cont.)  | Month         | Day           | Hour          |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Minute        | Second        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- `Year`: Current year.
- `Month`: Current month, 0-12.
- `Day`: Current day, 1-31.
- `Hour`: Current hour, 0-23.
- `Minute`: Current minute, 0-59.
- `Second`: Current second, 0-59.

#### `signpost_timelocation_get_location_reply(uint8_t destination_address, signpost_timelocation_location_t* location)`
Controller reply to location requesting module

parameters:
`destination_address`: I<sup>2</sup> address of requesting module
`location`: `signpost_timelocation_location_t` struct to return

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x06     | Type: 0x01    | Latitude
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         Latitude (cont.)                              | Longitude
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         Longitude (cont.)                             |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```


