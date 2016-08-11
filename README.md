Signpost
========

<img src="https://raw.githubusercontent.com/lab11/signpost/master/media/solar_panel_on_signpost6_cropped.jpg" alt="Signpost" width="10%" align="left" />

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


TODO
----

1. Case
  - [ ] Create a design we're happy with.
  - [ ] Prototype it in 3D printing.
  - [ ] Get 2-10 milled.

2. Power Supply
  - [x] Get baseline estimate of solar energy available.
  - [ ] Measure in realistic city street environment.
  - [ ] Create device to replicate a solar trace.
  - [ ] Design power supply to harvest, charge batteries, and provide VCC.

3. Control Board
  - [ ] Decide what we want on the control board.
  - [ ] Choose a MCU.
  - [ ] Choose a Linux SoM.
  - [ ] Create control board.

4. Modules
  - Ambient
    - [ ] Choose sensors.
    - [ ] Design ambient sensor module.
  - Audio
    - [ ] Decide what the module should do.
    - [ ] Implement it.
  - BLE
    - [ ] Provide BLE radio?
  - Wireless
    - [ ] LoRA?
    - [ ] GSM?

