Edison Support Files
====================

These are files that are included on the base edison image, but as also nice to
have around as easily accessible files for working with edisons in general.

 - `edison-i2c.sh` - The edison does not map the pins for userland I2C support
                     by default. This will set up the pins for `/dev/i2c-6`.
