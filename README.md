Signpost
========

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_on_sign_full_666x1000.jpg" alt="Signpost" width="31%" align="left" />
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_close_up_662x1000.jpg" align="left" width="31%" />

Modular city-scale sensing platform.

<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />

Architecture
------------

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
| Pulse per second. TODO: how is this driven? | PPS      | 7   |   | 8   | MOD_OUT  | Interrupt line to the controller. Allows modules to signal the controller.          |
|                                             | Reserved | 9   |   | 10  | MOD_IN   | GPIO from controller to module.                                                     |
| USB Data+ signal.                           | USB_D+   | 11  |   | 12  | USB_D-   | USB Data- signal.                                                                   |
| USB bus voltage (5 V).                      | USB_VBUS | 13  |   | 14  | GND      | USB GND.                                                                            |
