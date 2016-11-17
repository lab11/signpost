Signpost
========

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_on_sign_full_666x1000.jpg" alt="Signpost" width="31%" align="left" />
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_close_up_662x1000.jpg" align="left" width="31%" />

The Signpost project is a modular city-scale sensing platform that is installed on existing signposts, harvests
energy to power itself, and provides eight slots for diverse sensing modules that support
a range of applications. This significantly reduces the barrier for city-scale sensing
by not requiring a connection to AC mains power, providing shared resources (power, communication,
storage, and computation) for sensing modules, and supporting a programming environment
for building inter-signpost and inter-module applications.

<br />

Architecture
------------

Each Signpost platform includes a power supply that also meters energy usage, a controller that provides
storage and computation and manages the operation of the Signpost, and several module slots that support
extensible sensor modules for city-scale sensing applications. All modules are connected via an I2C bus,
and core system features (e.g. per-module energy metering) are on a SMBus network. For higher bandwidth
communication on the Signpost, each module is also attached to a USB 2.0 host.

Each module can be individually disconnected from the USB host and/or the I2C bus, as well as entirely
powered off.

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_arch_1000x445.jpg" />



Creating a Module
-----------------

To create a module for the signpost, it must conform to the module specification.

### Physical Dimensions

All in inches.

<a href="https://raw.githubusercontent.com/lab11/signpost/master/media/module_pcb_dimensions.png">
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/module_pcb_dimensions.png" width="50%" />
</a>

### Header Signals

| Description                                 | Signal   | Pin |   | Pin | Signal   | Description                                                                         |
|---------------------------------------------|----------|-----|---|-----|----------|-------------------------------------------------------------------------------------|
|                                             | GND      | 1   |   | 2   | 5V       |                                                                                     |
|                                             | Reserved | 3   |   | 4   | VCCIO    | I/O voltage the module uses. This must be fed by the module to set the I/O voltage. |
| I²C clock line.                             | SCL      | 5   |   | 6   | SDA      | I²C data line.                                                                      |
| Pulse per second from GPS.                  | PPS      | 7   |   | 8   | MOD_OUT  | Interrupt line to the controller. Allows modules to signal the controller.          |
|                                             | Reserved | 9   |   | 10  | MOD_IN   | GPIO from controller to module.                                                     |
| USB Data+ signal.                           | USB_D+   | 11  |   | 12  | USB_D-   | USB Data- signal.                                                                   |
| USB bus voltage (5 V).                      | USB_VBUS | 13  |   | 14  | GND      | USB GND.                                                                            |
