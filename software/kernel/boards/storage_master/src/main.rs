#![crate_name = "storage_master"]
#![no_std]
#![no_main]
#![feature(asm,const_fn,lang_items)]

extern crate cortexm4;
extern crate capsules;
#[macro_use(debug, static_init)]
extern crate kernel;
extern crate sam4l;

extern crate signpost_drivers;
extern crate signpost_hil;

use capsules::console::{self, Console};
use capsules::timer::TimerDriver;
use capsules::virtual_alarm::{MuxAlarm, VirtualMuxAlarm};
use kernel::hil;
use kernel::hil::Controller;
use kernel::hil::gpio::Pin;
use kernel::{Chip, Platform};
use kernel::mpu::MPU;
use sam4l::usart;

// For panic!()
#[macro_use]
pub mod io;


unsafe fn load_processes() -> &'static mut [Option<kernel::process::Process<'static>>] {
    extern "C" {
        /// Beginning of the ROM region containing app images.
        static _sapps: u8;
    }

    const NUM_PROCS: usize = 2;

    // how should the kernel respond when a process faults
    const FAULT_RESPONSE: kernel::process::FaultResponse = kernel::process::FaultResponse::Panic;
    #[link_section = ".app_memory"]
    static mut APP_MEMORY: [u8; 16384*2] = [0; 16384*2];

    static mut PROCESSES: [Option<kernel::process::Process<'static>>; NUM_PROCS] = [None, None];

    let mut apps_in_flash_ptr = &_sapps as *const u8;
    let mut app_memory_ptr = APP_MEMORY.as_mut_ptr();
    let mut app_memory_size = APP_MEMORY.len();
    for i in 0..NUM_PROCS {
        let (process, flash_offset, memory_offset) =
            kernel::process::Process::create(apps_in_flash_ptr,
                                             app_memory_ptr,
                                             app_memory_size,
                                             FAULT_RESPONSE);

        if process.is_none() {
            break;
        }

        PROCESSES[i] = process;
        apps_in_flash_ptr = apps_in_flash_ptr.offset(flash_offset as isize);
        app_memory_ptr = app_memory_ptr.offset(memory_offset as isize);
        app_memory_size -= memory_offset;
    }

    &mut PROCESSES
}

/*******************************************************************************
 * Setup this platform
 ******************************************************************************/

struct SignpostStorageMaster {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    led: &'static capsules::led::LED<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    sdcard: &'static capsules::sdcard::SDCardDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    rng: &'static capsules::rng::SimpleRng<'static, sam4l::trng::Trng<'static>>,
    ipc: kernel::ipc::IPC,
}

impl Platform for SignpostStorageMaster {
    fn with_driver<F, R>(&self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            8 => f(Some(self.led)),
            13 => f(Some(self.i2c_master_slave)),
            14 => f(Some(self.rng)),
            15 => f(Some(self.sdcard)),

            0xff => f(Some(&self.ipc)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA, PB};
    use sam4l::gpio::PeripheralFunction::{A, B};

    // configure SD card SPI as input
    PA[14].configure(None); // SD_SCLK
    PA[14].make_input();
    PA[15].configure(None); // SD_MISO
    PA[14].make_input();
    PA[16].configure(None); // SD_MOSI
    PA[14].make_input();
    PA[13].configure(None); // SD_CS
    PA[14].make_input();
    PA[21].configure(None); // SD_ENABLE

    // power off SD card
    PA[21].enable();
    PA[21].clear();
    PA[21].enable_output();

    // delay for a while
    asm!("nop");
    for _ in 0..200000 {
        asm!("nop");
    }
    asm!("nop");

    // power on SD card
    PA[21].enable();
    PA[21].set();
    PA[21].enable_output();

    // delay for a while
    asm!("nop");
    for _ in 0..200000 {
        asm!("nop");
    }
    asm!("nop");

    // SPI: Slave to Controller - USART0
    PA[10].configure(Some(A)); // MEMORY_SCLK
    PA[11].configure(Some(A)); // MEMORY_MISO
    PA[12].configure(Some(A)); // MEMORY_MOSI
    PA[09].configure(None); // !STORAGE_CS

    // SPI: Master of SD Card - USART1
    PA[14].configure(Some(A)); // SD_SCLK
    PA[15].configure(Some(A)); // SD_MISO
    PA[16].configure(Some(A)); // SD_MOSI
    PA[13].configure(None); // SD_CS
    PA[13].enable();
    PA[13].set();
    PA[13].enable_output();

    // GPIO: SD Card
    PA[17].configure(None); // !SD_DETECT
    PA[21].configure(None); // SD_ENABLE
    PA[21].enable();
    PA[21].set();
    PA[21].enable_output();

    // SPI: Slave to Edison - USART2
    PA[18].configure(Some(A)); // EDISON_SCLK
    PA[19].configure(Some(A)); // EDISON_MOSI
    PA[20].configure(Some(A)); // EDISON_MISO
    PA[22].configure(None); // !EDISON_SPI_CS

    // GPIO: Edison
    PA[05].configure(None); // !EDISON_PWRBTN
    PA[05].enable();
    PA[05].set();
    PA[05].enable_output();
    PA[06].configure(None); // LINUX_ENABLE_POWER
    PA[06].enable();
    PA[06].set();
    PA[06].enable_output();

    // I2C: Modules - TWIMS0
    PA[23].configure(Some(B)); // MODULES_SDA
    PA[24].configure(Some(B)); // MODULES_SCL

    // UART: Debug - USART3
    PB[09].configure(Some(A)); // STORAGE_DEBUG_RX
    PB[10].configure(Some(A)); // STORAGE_DEBUG_TX

    // GPIO: Debug
    PA[07].configure(None); // !STORAGE_LED
    PB[04].configure(None); // STORAGE_DEBUG_GPIO1
    PB[05].configure(None); // STORAGE_DEBUG_GPIO2
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    // Setup clock
    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);
    //sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillatorPll, 48000000);

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

    let console = static_init!(
        Console<usart::USART>,
        Console::new(&usart::USART3,
                     115200,
                     &mut console::WRITE_BUF,
                     kernel::Container::create()),
        224/8);
    hil::uart::UART::set_client(&usart::USART3, console);

    //
    // Timer
    //
    let ast = &sam4l::ast::AST;

    let mux_alarm = static_init!(
        MuxAlarm<'static, sam4l::ast::Ast>,
        MuxAlarm::new(&sam4l::ast::AST),
        16);
    ast.configure(mux_alarm);

    let virtual_alarm1 = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        24);
    let timer = static_init!(
        TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        TimerDriver::new(virtual_alarm1, kernel::Container::create()),
        12);
    virtual_alarm1.set_client(timer);

    // Setup RNG
    let rng = static_init!(
            capsules::rng::SimpleRng<'static, sam4l::trng::Trng>,
            capsules::rng::SimpleRng::new(&sam4l::trng::TRNG, kernel::Container::create()),
            96/8);
    sam4l::trng::TRNG.set_client(rng);

    //
    // I2C Buses
    //
    let i2c_modules = static_init!(
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver::new(&sam4l::i2c::I2C0,
            &mut capsules::i2c_master_slave_driver::BUFFER1,
            &mut capsules::i2c_master_slave_driver::BUFFER2,
            &mut capsules::i2c_master_slave_driver::BUFFER3),
        864/8);
    sam4l::i2c::I2C0.set_master_client(i2c_modules);
    sam4l::i2c::I2C0.set_slave_client(i2c_modules);

    // Set I2C slave address here, because it is board specific and not app
    // specific. It can be overridden in the app, of course.
    hil::i2c::I2CSlave::set_address(&sam4l::i2c::I2C0, 0x20);

    //
    // SD card interface, SPI master
    //
    let mux_spi = static_init!(
        capsules::virtual_spi::MuxSpiMaster<'static, usart::USART>,
        capsules::virtual_spi::MuxSpiMaster::new(&sam4l::usart::USART1),
        96/8);
    hil::spi::SpiMaster::set_client(&sam4l::usart::USART1, mux_spi);
    hil::spi::SpiMaster::init(&sam4l::usart::USART1);
    let sdcard_spi = static_init!(
        capsules::virtual_spi::VirtualSpiMasterDevice<'static, usart::USART>,
        capsules::virtual_spi::VirtualSpiMasterDevice::new(mux_spi, Some(&sam4l::gpio::PA[13])),
        384/8);
    let sdcard_virtual_alarm = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        192/8);
    let sdcard = static_init!(
        capsules::sdcard::SDCard<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        capsules::sdcard::SDCard::new(sdcard_spi, sdcard_virtual_alarm,
            Some(&sam4l::gpio::PA[17]), &mut capsules::sdcard::TXBUFFER, &mut capsules::sdcard::RXBUFFER),
        672/8);
    sdcard_spi.set_client(sdcard);
    sdcard_virtual_alarm.set_client(sdcard);
    sam4l::gpio::PA[17].set_client(sdcard);
    let sdcard_driver = static_init!(
        capsules::sdcard::SDCardDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        capsules::sdcard::SDCardDriver::new(sdcard, &mut capsules::sdcard::KERNEL_BUFFER),
        480/8);
    sdcard.set_client(sdcard_driver);

    //
    // Remaining GPIO pins
    //
    // Note: We're setting two fake GPIO pins to start off with. Currently,
    // signpost_api_init does some GPIO toggling and assumes what these pins
    // are, previously much to the dismay of the edison...
    let gpio_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 5],
        [&sam4l::gpio::PB[14], // Fake MOD_IN for init
         &sam4l::gpio::PB[15], // Fake MOD_OUT for init
         &sam4l::gpio::PA[05], // EDISON_PWRBTN
         &sam4l::gpio::PA[06],  // LINUX_ENABLE_POWER
         &sam4l::gpio::PA[21]], // SD_ENABLE
        5 * 4
    );
    let gpio = static_init!(
        capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
        capsules::gpio::GPIO::new(gpio_pins),
        224/8);
    for pin in gpio_pins.iter() {
        pin.set_client(gpio);
    }

    //
    // LEDs
    //
    let led_pins = static_init!(
        [(&'static sam4l::gpio::GPIOPin, capsules::led::ActivationMode); 3],
        [(&sam4l::gpio::PB[04], capsules::led::ActivationMode::ActiveHigh), // STORAGE_DEBUG_GPIO1
         (&sam4l::gpio::PB[05], capsules::led::ActivationMode::ActiveHigh), // STORAGE_DEBUG_GPIO2
         (&sam4l::gpio::PA[07], capsules::led::ActivationMode::ActiveLow),  // STORAGE_LED
        ],
        192/8);
    let led = static_init!(
        capsules::led::LED<'static, sam4l::gpio::GPIOPin>,
        capsules::led::LED::new(led_pins),
        64/8);

    // configure initial state for debug LEDs
    sam4l::gpio::PB[04].clear(); // red LED off
    sam4l::gpio::PB[05].set();   // green LED on

    //
    // Actual platform object
    //
    let signpost_storage_master = SignpostStorageMaster {
        console: console,
        gpio: gpio,
        led: led,
        timer: timer,
        i2c_master_slave: i2c_modules,
        sdcard: sdcard_driver,
        rng: rng,
        ipc: kernel::ipc::IPC::new(),
    };

    signpost_storage_master.console.initialize();
    // Attach the kernel debug interface to this console
    let debug_console = static_init!(
        capsules::console::App,
        capsules::console::App::default(),
        480/8);
    kernel::debug::assign_console_driver(Some(signpost_storage_master.console), debug_console);

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    debug!("Board setup complete");
    kernel::main(&signpost_storage_master, &mut chip, load_processes(), &signpost_storage_master.ipc);
}
