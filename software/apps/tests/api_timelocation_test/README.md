API: Time Location Test
===============

This test requires a Signpost with a Controller.

It is designed to run on any module.


Example output
--------------

```
[Test] API: Time & Location
Query Time
WARN: Encryption key lookup for I2C address 0x20 failed.
      This will likely result in a HMAC failure and a message drop.
RECV ENCRYPTED: 0
  Current time: 2080/1/6 3:14:4
Query Location
WARN: Encryption key lookup for I2C address 0x20 failed.
      This will likely result in a HMAC failure and a message drop.
RECV ENCRYPTED: 0
  Current location:
    Latitude:
    Longitude:
Sleeping for 5s
```
