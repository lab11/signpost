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
       |Version| Flags | Source        |0| Length                      |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       | Offset                          | ... 
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### Version

The protocol version in use. Currently, the only valid version is version 1.

### Flags

  - Bit 4: Reserved
  - Bit 5: Reserved
  - Bit 6: Reserved
  - Bit 7: IsFragment - 1 if message has more data, 0 if end of message

### Source

The 7 bit source address of the device sending this message.

### Length

The length of the complete message in bytes.

### Offset

The offset into the complete message in bytes. This field is used to support fragmentation.


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

XXX TODO: Link each to their doc's

  - `0x01`: Initialization
  - `0x02`: Storage
  - `0x03`: Networking
  - `0x04`: Processing
  - `0x05`: Energy
  - `0x06`: Time and Location


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
