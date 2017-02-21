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

TODO finish filling this out
Sent Message:

```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x03     | Type: 0x00    | URLlen (LSB)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         URLlen (MSB)  | URL           | Day           | Hour          |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Minute        | Second        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Response Message:
```text
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Frame: 0x01   | API: 0x03     | Type: 0x00    | URLlen (LSB)
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         URLlen (MSB)  | URL           | Day           | Hour          |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Minute        | Second        |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```


### `0x04`: Processing

### `0x05`: Energy

### `0x06`: Time and Location

#### `get_time()`

Response message:

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

#### `get_location()`

Response message:

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

- `Latitude`: Latitude in microdegrees. Divide by 10^6 to get actual degrees.
- `Longitude`: Longitude in microdegrees. Divide by 10^6 to get actual degrees.



