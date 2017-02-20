#!/bin/sh

# Enable I2C on Intel Edison Arduino breakout board. The Linux device name
# is /dev/i2c-6 on Arduino hardware pins A4 (SDA) and A5 (SCL).

# Modified from
# https://github.com/bbx10/htu21dflib/blob/master/edison-i2c.sh

# Which in turn had modified
# http://www.emutexlabs.com/project/215-intel-edison-gpio-pin-multiplexing-guide

echo 14 > /sys/class/gpio/export
echo 165 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio14/direction
echo in > /sys/class/gpio/gpio165/direction
echo mode1 > /sys/kernel/debug/gpio_debug/gpio28/current_pinmux
echo mode1 > /sys/kernel/debug/gpio_debug/gpio27/current_pinmux


## Preserving these b/c the first of the borrowed from links already died,
## but we don't use these pins (think they're unique to the magic Arduino config)

#echo 28 > /sys/class/gpio/export
#echo 27 > /sys/class/gpio/export
#echo 204 > /sys/class/gpio/export
#echo 205 > /sys/class/gpio/export
#echo 236 > /sys/class/gpio/export
#echo 237 > /sys/class/gpio/export
#echo 212 > /sys/class/gpio/export
#echo 213 > /sys/class/gpio/export
#echo 214 > /sys/class/gpio/export
