Signpost
========

[![Build Status](https://travis-ci.org/lab11/signpost.svg?branch=master)](https://travis-ci.org/lab11/signpost)

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_on_sign_full_666x1000.jpg" alt="Signpost" width="31%" align="left" />
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_close_up_662x1000.jpg" align="left" width="31%" />

The Signpost project is a modular city-scale sensing platform that is installed on existing signposts, harvests
energy to power itself, and provides eight slots for diverse sensing modules that support
a range of applications. This significantly reduces the barrier for city-scale sensing
by not requiring a connection to AC mains power, providing shared resources (power, communication,
storage, and computation) for sensing modules, and supporting a programming environment
for building inter-signpost and inter-module applications.

The project is driven by several core applications, but also strives to be
an upgradeable and adaptable platform that supports new applications
for scientists and cities. Modules can be added and removed from the platform
after deployment without disassembling the installed Signpost, reprogramming
the Signpost, or interrupting the other functions of the Signpost. Additionally,
by providing APIs to modules that support common operations, developing and
deploying a sensing application in a city is significantly streamlined. A focus
of this project is ensuring that domain scientists and researchers interested
in city-scale applications can effectively leverage this platform to accelerate
their projects.


<br />

Getting Started
---------------

Clone this repo then run a `git submodule update --init --recursive`.

After that, follow the instructions for [flashing individual modules](software/kernel/boards/).


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

### Debug Header Signals

Modules may choose to implement a Debug header as well. The Debug header will allow for easier interactions with modules while they are connected to the development version of the backplane.

| Description                                         | Signal   | Pin |   | Pin | Signal   | Description                                                                         |
|-----------------------------------------------------|----------|-----|---|-----|----------|-------------------------------------------------------------------------------------|
|                                                     | GND      | 1   |   | 2   | GPIO1    | Output line to control LED on development backplane                                 |
| UART data from the module                           | TX       | 3   |   | 4   | RX       | UART data to the module                                                             |
| Output line to control LED on development backplane | GPIO2    | 5   |   | 6   | !Reset   | Active-low reset for the module microcontroller                           |
| JTAG line for the module microcontroller            | SWDCLK   | 7   |   | 8   | SWDIO    | JTAG line for the module microcontroller                                             |

Roadmap
-------

Developing the Signpost platform is an ongoing effort with several primary
goals:

- Designing a programming model for running applications across a network of
Signposts. This should truly simplify creating interesting and useful applications,
and not discourage development by imposing unnecessary hurdles.
- Developing the Signpost platform operation, including energy-sharing and
reliability ensurance.
- Creating a HW/SW test framework for accelerating module development.
- A deployment on the order of 10 devices by October 2017.


### History

- **Winter 2017**: Signpost v0.2 released for the
[TerraSwarm Signpost Workshop](https://www.terraswarm.org/urbanheartbeat/wiki/Main/SignpostWorkshop).
This workshop featured the release of the Debug Backplane, initial API
implementations for signpost modules, and tutorials for getting started with
the Signpost platform.
- **Fall 2016**: Signpost v0.1 presented at [TerraSwarm Annual Review](https://www.terraswarm.org/conferences/16/annual/).
The demo included six modules (ambient conditions, 2.4 GHz RF sensing, LoRa/BLE radios,
ambient audio level, microwave radar, and air quality sensing from UCSD), data communication
over LoRa to a gateway, and a real-time UI.
- **Summer 2016**: Discussions on physical design yield a prototype enclosure and module form factor.



Related Projects
----------------

City-scale sensing platforms are a growing area of research with several emerging
approaches:

- Chicago's [Array of Things](https://arrayofthings.github.io/)
- NYC's [SONYC](https://wp.nyu.edu/sonyc/)


License
-------

Licensed under either of

 * Apache License, Version 2.0
   ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
 * MIT license
   ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
