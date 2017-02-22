Signpost Tutorial
=================

## Get the basics working

### Toolchain setup
1) Clone repository

```bash
git clone --recursive -j8 https://github.com/lab11/signpost
```

1) Pull submodules and repo

```bash
cd signpost/
git pull
git submodule update --init --recursive
```

1) Rust

See [Getting Started](https://github.com/helena-project/tock/blob/master/doc/Getting_Started.md)

```bash
rustc --version
```
must be equal `rustc 1.16.0-nightly (83c2d9523 2017-01-24)`

2) arm-none-eabi-gcc

See [Getting Started](https://github.com/helena-project/tock/blob/master/doc/Getting_Started.md)

```bash
arm-none-eabi-gcc
```
must be >= 5.2

3) JLinkExe

You will need to install the
[JLink](https://www.segger.com/jlink-software.html) software for your platform.
You want the "Software and documentation pack".

```bash
JLinkExe --version
```
must be greater than 5

4) tockloader

```bash
sudo pip3 install tockloader
```

5) signpost-debug-radio

```bash
sudo pip2 install signpost-debug-radio
```

6) Check that you can compile a board

```bash
cd signpost/software/kernel/boards/controller/
make
```

7) Check that you can compile an app

```bash
cd signpost/software/apps/tock_examples/blink/
make
```


### Hardware setup

1) Collect parts
 * Debug Backplane
 * JLink programmer
 * 2x USB micro cables
 * 5v power supply
 * Control Module (with Intel Edison and SD card)
 * Audio Module
 * Ambient Module

2) Attach JLink programmer

3) Plug in Control Module

4) Apply power


### Controller Kernel

1) Turn knob to `MAIN`

2) Flash the kernel

```bash
cd signpost/software/kernel/boards/controller/
make flash
```


### Blink app

1) Turn knob to `MAIN`


2) Flash the application

```bash
cd signpost/software/apps/tock_examples/blink
make flash
```

The board should now have a blinking LED on the Control Module and two blinking
LEDs near the USB ports.


### Hello app

1) Turn knob to `MAIN`

2) Connect USB cable to `Controller Main` USB port

3) Connect to serial port

First open a new terminal window.

```bash
tockloader listen -d controller
```

This will open a serial terminal to the Controller. Note that nothing will print yet.

4) Flash the application

```bash
cd signpost/software/apps/tock_examples/c_hello/
make flash
```

"Hello World" should now have printed to the serial terminal.

5) Press the `Controller Main` reset button

"Hello World" should print again!


## Testing the Signpost APIs


## Ambient Module


## Multiple Modules


## Working on the Edison

