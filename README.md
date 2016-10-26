Signpost
========

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/solar_panel_on_signpost6_cropped.jpg" alt="Signpost" width="10%" align="left" />
<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/signpost_in_ann_arbor.jpg" align="left" width="75%" />

Modular city-scale sensing platform.


Architecture
------------

<img src="http://www.gliffy.com/go/publish/image/10989855/L.png" />



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
