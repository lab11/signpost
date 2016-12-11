#!/bin/sh

echo 114 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio114/direction
