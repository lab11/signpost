Signpost Tutorial
=================

## Dealing with problems

You will definitely run into problems. This is research hardware and
software still in its infancy. We really appreciate your patience.

If everything is not working, try resetting other modules and then the
`Controller Main`. Power cycling the entire board is a valid option too.

Module Initialization can take several seconds to complete. The Controller
should print the following once Initialization for a module is complete.

```
Module 0 granted isolation
INIT: Registered address 0x51 as module 0
```

The Green Debug LED should be on for both modules. The Red Debug LED should
be on before Initialization has completed and off afterwards. A blinking
pattern on the Red Debug LED signifies that the application has crashed.

Let us know if you're having problems and we will help out.


## Get the basics working

### Toolchain setup
0. Clone repository

    ```bash
    git clone --recursive -j8 https://github.com/lab11/signpost
    ```

1. Pull submodules and repo

    ```bash
    cd signpost/
    git pull
    git submodule update --init --recursive
    ```

2. Rust

    See [Getting Started](https://github.com/helena-project/tock/blob/master/doc/Getting_Started.md)

    ```bash
    rustc --version
    ```
    must be print `rustc 1.16.0-nightly (83c2d9523 2017-01-24)` if you've got everything set up correctly.

3. arm-none-eabi-gcc

    See [Getting Started](https://github.com/helena-project/tock/blob/master/doc/Getting_Started.md)

    ```bash
    arm-none-eabi-gcc --version
    ```
    must be >= 5.2

3. JLinkExe

    You will need to install the
    [JLink](https://www.segger.com/jlink-software.html) software for your platform.
    You want the "Software and documentation pack".

    ```bash
    JLinkExe --version
    ```
    must be greater than 5

4. tockloader

    ```bash
    sudo pip3 install tockloader
    ```

5. signpost-debug-radio

    ```bash
    sudo pip2 install signpost-debug-radio
    ```

6. Check that you can compile a board

    ```bash
    cd signpost/software/kernel/boards/controller/
    make
    ```

7. Check that you can compile an app

    ```bash
    cd signpost/software/apps/tock_examples/blink/
    make
    ```


### Hardware setup

1. Collect parts

 * Debug Backplane
 * JLink programmer
 * 2x USB micro cables
 * 5v power supply
 * Control Module
    - With an Intel Edison plugged into the back
    - With an SD card in the slot on the back
 * Audio Module
 * Ambient Module

2. Attach JLink programmer

3. Plug in Control Module

4. Apply power


### Controller Kernel

1. Turn knob to `MAIN`

2. Flash the kernel

    ```bash
    cd signpost/software/kernel/boards/controller/
    make flash
    ```


### Blink app

1. Turn knob to `MAIN`


2. Flash the application

    ```bash
    cd signpost/software/apps/tock_examples/blink
    make flash
    ```

    The board should now have a blinking LED on the Control Module and two blinking
    LEDs near the USB ports.

    ![Backplane with LEDs identified](img/controller_blink.jpg)


### Hello app

1. Turn knob to `MAIN`

2. Connect USB cable to `Controller Main` USB port

3. Connect to serial port

    First open a new terminal window.

    ```bash
    tockloader listen -d controller
    ```

    This will open a serial terminal to the Controller. Note that nothing will print yet.
    
    > Problem? Try `pip install --upgrade tockloader`. Should be at least v0.4.0

4. Flash the application

    **Note:** we do not need to re-flash the Controller kernel. The kernel can
    remains on the microcontroller even as apps change.

    ```bash
    cd signpost/software/apps/tock_examples/c_hello/
    make flash
    ```

    "Hello World" should now have printed to the serial terminal.

5. Press the `Controller Main` reset button

    "Hello World" should print again!


## Playing with Tock

### What's available to Tock applications

In C, Tock applications have access to most of the [standard library
functions](http://www.cplusplus.com/reference/clibrary/). Uncommonly for
embedded systems, this includes `printf` and `malloc`/`free`. There are also
many drivers that interact with kernel code through system calls. Which
particular drivers are available on a given platform is defined in its board
`main.rs` file. For example: the [syscall connections for the ambient
board](https://github.com/lab11/signpost/blob/master/software/kernel/boards/ambient_module/src/main.rs#L91).

1. Turn knob to `MAIN`

2. Connect USB cable to `Controller Main` USB port

3. Connect to serial port

    First open a new terminal window.

    ```bash
    tockloader listen -d controller
    ```

    This will open a serial terminal to the Controller. Note that nothing will print yet.
    
    > Problem? Try `pip install --upgrade tockloader`. Should be at least v0.4.0

4. Make a new application

    ```bash
    cd signpost/software/apps/
    mkdir hello_app
    cp template/Makefile hello_app/
    touch hello_app/main.c
    cd hello_app/
    ```

5. Write code

    Open `main.c` with your favorite text editor and make it match the following:

    ```c
    #include <stdio.h>

    int main (void) {
	    printf("Starting test\n");

	    return 0;
    }
    ```

4. Flash the application

    ```bash
    make flash
    ```

    The string "Starting test" should now have printed to the serial terminal.

5. Make it repeat

    Open `main.c` with your favorite text editor and make it match the
    following and then flash it:

    ```c
    //standard library
    #include <stdio.h>
    #include <stdint.h>

    // tock library
    #include <timer.h>

    int main (void) {
	    printf("Starting test\n");

	    while (true) {
		    printf("Hello!\n");
		    delay_ms(1000);
	    }
    }
    ```

    After flashing, the string "Hello" should be printed at once second
    intervals.

### Catching application faults

The biggest advantage of using the Tock kernel is application safety. Before
starting an application, the kernel configures the microcontroller's Memory
Protection Unit (MPU) to limit the memory accesses of the app. In practice this
means that invalid memory accesses are caught by the kernel and reported to
users.


1. Make a new application

    ```bash
    cd signpost/software/apps/
    mkdir fault_test
    cp template/Makefile fault_test
    touch fault_test/main.c
    cd fault_test/
    ```

2. Write code

    Edit `main.c` to match the following and then flash it:

    ```c
    #include <stdio.h>

    int main (void) {
	    printf("Starting test\n");

	    // dereferencing null pointer
	    volatile int i = *(int*)0;

	    return 0;
    }
    ```

    A fault report should have printed.


3. Examine fault report

    The printed fault report should look a lot like this:

    ```
    Kernel panic at /home/brghena/Dropbox/repos/signpost/software/kernel/tock/kernel/src/process.rs:283:
	"Process fault_test had a fault"

    ---| Fault Status |---
    Data Access Violation:              true
    Forced Hard Fault:                  true
    Faulting Memory Address:            0x00000000
    Fault Status Register (CFSR):       0x00000082
    Hard Fault Status Register (HFSR):  0x40000000

    ---| App Status |---
    App: fault_test   -   [Fault]
     Events Queued: 0   Syscall Count: 8   Last Syscall: YIELD

     ╔═══════════╤══════════════════════════════════════════╗
     ║  Address  │ Region Name    Used | Allocated (bytes)  ║
     ╚0x2000A000═╪══════════════════════════════════════════╝
		 │ ▼ Grant         324 |   1024          
      0x20009EBC ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈
		 │ Unused
      0x200076C8 ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈
		 │ ▲ Heap         1520 |   8192                      S
      0x200070D8 ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ R
		 │ Data            216 |    216                      A
      0x20007000 ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ M
		 │ ▼ Stack         304 |   4096          
      0x20006ED0 ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈
		 │ Unused
      0x20006000 ┴───────────────────────────────────────────
		 .....
      0x00031000 ┬───────────────────────────────────────────
		 │ Unused
      0x00030DD0 ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ F
		 │ Data            198                               L
      0x00030D0A ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ A
		 │ Text           3214                               S
      0x0003007C ┼┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ H
		 │ Header          124
      0x00030000 ┴───────────────────────────────────────────

      R0 : 0x0000000A    R6 : 0x00000000
      R1 : 0x20007054    R7 : 0x00000000
      R2 : 0x20007000    R8 : 0x00000000
      R3 : 0x00000000    R10: 0x00000000
      R4 : 0x00000000    R11: 0x00000000
      R5 : 0x00000000    R12: 0x00000000
      R9 : 0x20007000 (Static Base Register)
      SP : 0x20006FC8 (Process Stack Pointer)
      LR : 0x00030C5F [0x80000BE2 in lst file]
      PC : 0x0003009A [0x8000001E in lst file]
     YPC : 0x000300A8 [0x8002F41A in lst file]
    ```

    * `Data Access Violation` means the app attempted to access a memory
      address it is not allowed to.

    * `Faulting Memory Address` is the address the app attempted to access.

    * `SRAM` is the volatile read/write memory space on the microcontroller.
      The application's data segment, stack, and heap live there.

    * `FLASH` is the non-volatile read-only memory space on the
      microcontroller. The application's code segment and initial data values
      live there.

### Bonus: Running two applications concurrently

Tock enables running multiple applications concurrently on a microcontroller.
The applications are handled independently and protected from each other. Many
Tock drivers are virtualized so that they can be used by multiple applications
simultaneously.

**Warning:** this functionality is relatively new and still rough. In some cases
running multiple applications will result in odd bugs and failures or simply
not fit in memory at all. While this capability is key part of future Signpost
(and Tock) development, currently use at your own risk.

1. Build the blink app

    ```bash
    cd signpost/software/apps/tock_examples/blink/
    make
    ```

2. Build your hello app

    ```bash
    cd signpost/software/apps/hello_app/
    make
    ```

3. Upload both apps

    ```bash
    cd signpost/software/
    kernel/tock/userland/tools/flash/storm-flash-app.py apps/fault_test/build/audio_module/app.bin apps/tock_examples/blink/build/storm/app.bin 
    ```

    Both apps should now be loaded on the module. The LEDs should blink and
    "Hello" should be printing to the serial terminal.


## Testing the Signpost APIs

[API Documentation](https://github.com/lab11/signpost/blob/master/software/docs/ApiGuide.md)

### Set up the Signpost platform

1. Flash the Storage Master kernel

    **Important** Make sure the programming knob is turned to `MEM`.

    ```bash
    cd signpost/software/kernel/boards/storage_master/
    make flash
    ```

2. Flash the Signpost Storage Master app

    ```bash
    cd signpost/software/apps/storage_master/signpost_storage_master
    make flash
    ```

3. View Storage Master output

    Connect a USB cable to the `Memory` USB plug.

    ```bash
    tockloader listen -d storage
    ```

    Hit the `Memory` reset button. (Near the USB plug)

    A message should print that the Storage Master has detected an SD card and
    initialized it.

3. Flash the Signpost Controller app

    **Important** Make sure the programming knob is turned to `MAIN`.

    ```bash
    cd signpost/software/apps/controller/signpost_controller_app/
    make flash
    ```

4. View Controller output

    Connect a USB cable to the `Main` USB plug.

    ```bash
    tockloader listen -d controller
    ```

    Hit the `Controller Main` reset button.

    The Controller should print GPS updates once per second. GPS doesn't get a
    fix indoors (or without an antenna) unfortunately, but the GPS module
    starts with a default timestamp which it updates once per second.
    ```
    GPS Data: 1:56:02.2000 1/6/80
      Latitude:   0 degrees
      Longitude:  0 degrees
      Status:     Invalid fix
      Satellites: 0
    ```

    **Note:** we do have some GPS antennas with long cables which you can test
    with if they are not in use. Flag us down.

    Once every ten seconds it prints an Energy use update:
    ```
    Energy Data
      Module 0 energy:            0 uAh
      Module 1 energy:            0 uAh
      Controller energy:       5624 uAh
      Linux energy:               0 uAh
    ```

5. Flash the Debug Radio Module kernel

    **Important** Make sure the programming knob is turned to `RADIO`.

    ```bash
    cd signpost/software/kernel/boards/debug_radio_module/
    make flash
    ```

6. Flash the Signpost Debug Radio Module app

    ```bash
    cd signpost/software/apps/debug_radio_module/signpost_debug_radio_app/
    make flash
    ```

### Set up the Ambient Module

1. Plug the Ambient Module into the Module 0 slot

2. Flash the Ambient Module kernel

    **Important** Make sure the programming knob is turned to `MOD0`.

    ```bash
    cd signpost/software/kernel/boards/ambient_module/
    make flash
    ```

3. Flash the hello app

    ```bash
    cd signpost/software/apps/hello_app/
    make flash
    ```

4. View Ambient output

    Connect a USB cable to the `Module 0` USB plug.

    ```bash
    tockloader listen -d module0
    ```

    Hit the `Module 0` reset button.

### Test the Time and Location API

[Time and Location API docs](https://github.com/lab11/signpost/blob/master/software/docs/ApiGuide.md#time)

1. Look at the example code

    [API Time and Location Test](https://github.com/lab11/signpost/blob/master/software/apps/tests/api_timelocation_test/main.c)

2. Flash the Time and Location test app

    **Important** Make sure the programming knob is turned to `MOD0`.

    ```bash
    cd signpost/software/apps/tests/api_timelocation_test/
    make flash
    ```

    Hit the `Module 0` reset button.

3. Open serial connections

    Connect USB cables to both the `Controller Main` and `Module 0` USB plugs.

    In one terminal window:
    ```bash
    tockloader listen -d controller
    ```

    In another terminal window:
    ```bash
    tockloader listen -d module0
    ```

4. Data output

    If everything is successful, every five seconds `Module 0` should print
    something resembling:
    ```
    Query Time
      Current time: 2080/1/6 3:38:46 with 0 satellites
    Query Location
      Current location:
	Latitude:   0
	Longitude:  0
      With 0 satellites
    Sleeping for 5s
    ```

### Test the Storage API

[Storage API docs](https://github.com/lab11/signpost/blob/master/software/docs/ApiGuide.md#time)

1. Look at the example code

    [API Storage Test](https://github.com/lab11/signpost/blob/master/software/apps/tests/api_storage_test/main.c)

2. Flash the Storage test app

3. View Ambient output

4. View the Storage Master output

### Test the Networking API

[Networking API docs](https://github.com/lab11/signpost/blob/master/software/docs/ApiGuide.md#networking)

1. Look at the example code

    **XXX: NEED TO LINK THIS**
    [API Simple Networking Test]()

2. Flash the Networking test app

3. View Ambient output

4. Start the Debug Radio script







## Ambient Module


## Multiple Modules


## Working on the Edison

