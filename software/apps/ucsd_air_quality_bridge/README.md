UCSD Air Quality Bridge
=======================

The UCSD Air Quality sensor generates a decent amount of I2C traffic to talk to
its sensors. While that in principle could join with the backplane I2C network,
it's not a great idea. Unfortunately, the Photon only has one I2C bus, so we
write data via UART to this app, which relays it onto the signpost I2C network.

**Program with `audio_module_test` kernel**

