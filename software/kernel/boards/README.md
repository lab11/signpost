Platforms Supported by Signpost
===============================

This folder contains kernel build files for boards supported by the Signpost
project.

Each Board folder contains Makefiles for controlling the kernel build and app
builds, Rust Cargo files to control the Rust build process, and a `src/`
folder. The `src/` folder contains a `main.rs` file which connects all pins on
the microcontroller to specific drivers, provides memory for applications, and 
initializes the platform. There is also an `io.rs` file in `src/` which has
code enabling the board to print error messages.

Setup
-----

Follow the Tock [Getting Started](https://github.com/helena-project/tock/blob/master/doc/Getting_Started.md)
to install Rust and GCC.

Programming Signpost modules requires a [JLink JTAG programmer](https://www.segger.com/jlink-general-info.html).
The "EDU" edition works fine.

You will need to install the JLink [software](https://www.segger.com/jlink-software.html)
for your platform. You want the "Software and documentation pack".


Programming the Tock Kernel and Apps
------------------------------------

To program a kernel for Signpost with JTAG connected to the module:

```bash
cd software/kernel/boards/<BOARD_NAME>/
make flash
```

To program an application:

```bash
cd software/apps/<BOARD_NAME>/<APP_NAME>/
make flash
```

