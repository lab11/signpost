Signpost Hardware
=================

This folder collects all of the core hardware that makes up the signpost platform.
For modules made by Lab11, visit the [modules folder](../modules).


SMBus Addresses
---------------
_Private bus for controller / power / backplane_

### Backplane
| Thing                | Part Number | Address (bin) | Address (hex) |
|----------------------|-------------|---------------|---------------|
| GPIO Extender Mod0   | MCP23008    | 0100000       | 0x20          |
| GPIO Extender Mod1   | MCP23008    | 0100001       | 0x21          |
| GPIO Extender Mod2   | MCP23008    | 0100010       | 0x22          |
| GPIO Extender USB    | MCP23008    | 0100011       | 0x23          |
| GPIO Extender Mod5   | MCP23008    | 0100101       | 0x25          |
| GPIO Extender Mod6   | MCP23008    | 0100110       | 0x26          |
| GPIO Extender Mod7   | MCP23008    | 0100111       | 0x27          |
| EEPROM ID            | M24C01      | 1010001       | 0x51          |


### Power
| Thing                         | Part Number | Address (bin) | Address (hex) |
|-------------------------------|-------------|---------------|---------------|
| SMBUS Switch 0                | PCA9544A    | 1110000       | 0x70          |
| SMBUS Switch 1                | PCA9544A    | 1110001       | 0x71          |
| SMBUS Switch 2                | PCA9544A    | 1110001       | 0x72          |
| Battery Monitor Addr[000:0FF] | MAX17205    | 0110110       | 0x36          |
| Battery Monitor Addr[100:17F] | MAX17205    | 0001011       | 0x0B          |
| Module Coulomb Counter (x8)   | LTC2943     | 1100100       | 0x64          |
| Solar Coulomb Counter         | LTC2943     | 1100100       | 0x64          |
| EEPROM ID                     | M24C01      | 1010000       | 0x50          |


Signpost I2C Network
--------------------
_The shared I2C bus that runs to each module_

### Controller
| Thing                     | Address(es) |
|---------------------------|-------------|
| Controller                | 0x20        |
| Storage                   | 0x21        |

### Modules
| Thing                     | Address(es) |
|---------------------------|-------------|
| Radio Module              | 0x22        |
| Scanner154 Module         | 0x31        |
| Ambient Sensing Module    | 0x32        |
| Audio Sensing Module      | 0x33        |
| Radar Module              | 0x34        |
| Air Quality Module (UCSD) | 0x35        |
