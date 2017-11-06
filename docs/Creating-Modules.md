Creating Signpost Modules
=========================

To create a module for the signpost, it must conform to the module specification.

Physical Dimensions
-------------------

All in inches.

<a href="https://raw.githubusercontent.com/lab11/signpost/master/media/module_pcb_dimensions.png">
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/module_pcb_dimensions.png" width="50%" />
</a>

Header Signals
--------------

| Description                                 | Signal   | Pin |   | Pin | Signal   | Description                                                                         |
|---------------------------------------------|----------|-----|---|-----|----------|-------------------------------------------------------------------------------------|
|                                             | GND      | 1   |   | 2   | 5V       |                                                                                     |
|                                             | Reserved | 3   |   | 4   | VCCIO    | I/O voltage the module uses. This must be fed by the module to set the I/O voltage. |
| I²C clock line.                             | SCL      | 5   |   | 6   | SDA      | I²C data line.                                                                      |
| Pulse per second from GPS.                  | PPS      | 7   |   | 8   | MOD_OUT  | Interrupt line to the controller. Allows modules to signal the controller.          |
|                                             | Reserved | 9   |   | 10  | MOD_IN   | GPIO from controller to module.                                                     |
| USB Data+ signal.                           | USB_D+   | 11  |   | 12  | USB_D-   | USB Data- signal.                                                                   |
| USB bus voltage (5 V).                      | USB_VBUS | 13  |   | 14  | GND      | USB GND.                                                                            |

Debug Header Signals
-------------------

Modules may choose to implement a Debug header as well. The Debug header will allow for easier interactions with modules while they are connected to the development version of the backplane.

| Description                                         | Signal   | Pin |   | Pin | Signal   | Description                                                                         |
|-----------------------------------------------------|----------|-----|---|-----|----------|-------------------------------------------------------------------------------------|
|                                                     | GND      | 1   |   | 2   | GPIO1    | Output line to control LED on development backplane                                 |
| UART data from the module                           | TX       | 3   |   | 4   | RX       | UART data to the module                                                             |
| Output line to control LED on development backplane | GPIO2    | 5   |   | 6   | !Reset   | Active-low reset for the module microcontroller                           |
| JTAG line for the module microcontroller            | SWDCLK   | 7   |   | 8   | SWDIO    | JTAG line for the module microcontroller                                             |


Existing Hardware Libraries and References
-----------------------------------------

We have built sensor modules using both KiCAD and EAGLE. Generic
module libraries that include a reference outline and header location
are available in the [Signpost Eagle Library](github.com/lab11/signpost/blob/master/hardware/signpost.lbr).

All example sensor modules can be found in the [modules folder](github.com/lab11/signpost/tree/master/modules).

Tips 
----

We generally recommend using a processor supported by [Tock](github.com/helena-project/tock)
or [ARM Mbed](mbed.com).

Holes in the sensor modules that allow it to screw to the signpost
backplane should be plated and tied to ground.

Module Review
-------------

We are happy to provide guidance
on module creation! Feel free to reach out to us either through
an issue or email at <signpost-admin@eecs.berkeley.edu>
