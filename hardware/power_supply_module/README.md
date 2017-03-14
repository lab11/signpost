Power Supply Module
===================

XXX A picture would be super nice here XXX _Neal_ XXX

The power module monitors and supplies energy for all signpost subsystems.
It tracks the amount of energy collected from the solar panel and monitors
the energy consumed by each module. The power module can be configured
to alert (via `!SMBALERT`) when a module or subsystem draws an excess amount
of energy, however the controller must issue commands to cut power if needed.
In extreme cases (i.e. shorts), the power module does include resettable fuses
for protection.

Physical Layout
---------------
The Power Module has one header that it uses to interact with the Backplane: a 20-pin header on top.

| Description                          | Signal     | Pin  | Pin  | Signal     | Description                                 |
|--------------------------------------|------------|------|------|------------|---------------------------------------------|
| Ground                               | GND        | `01` | `02` | VCC_Cont   | VCC for Controller and Storage Master (5V)  |
| SMBus Data Line                      | SMBData    | `03` | `04` | VCC_Linux  | VCC for Linux (5V)                          |
| SMBus Clock Line                     | SMBCLK     | `05` | `06` | !SMBALERT  | SMBus Alert line                            |
| VCC for Module 0 (5V)                | VCC_MOD0   | `07` | `08` | VCC_MOD1   | VCC for Module 1 (5V)                       |
| VCC for Module 2 (5V)                | VCC_MOD2   | `09` | `10` | VCC_MOD5   | VCC for Module 5 (5V)                       |
| VCC for Module 6 (5V)                | VCC_MOD6   | `11` | `12` | VCC_MOD7   | VCC for Module 7 (5V)                       |
| VCC for Backplane (3.3V)             | VCC_BACK   | `13` | `14` | _Reserved_ |                                             |
|                                      | _Reserved_ | `15` | `16` | !WATCHDOG  | Watchdog input (active low, from Backplane) |
| Positive connection from battery     | VBATT+     | `17` | `18` | VBATT-     | Negative connection from battery            |
| Positive connection from solar panel | VSOL+      | `19` | `20` | VSOL-      | Negative connection from solar panel        |


Requirements / Assumptions
--------------------------

XXX Neal

No battery -> no power.
Battery volatage range?

Solar voltage range?
