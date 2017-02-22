#![crate_name = "audio_module"]
#![no_std]
#![no_main]
#![feature(asm,const_fn,lang_items)]

extern crate cortexm4;
extern crate capsules;
#[macro_use(debug,static_init)]
extern crate kernel;
extern crate sam4l;

extern crate signpost_drivers;
extern crate signpost_hil;

// use capsules::console::{self, Console};
use capsules::timer::TimerDriver;
use capsules::virtual_alarm::{MuxAlarm, VirtualMuxAlarm};
use kernel::hil;
use kernel::hil::Controller;
use kernel::{Chip, Platform};
use kernel::mpu::MPU;
use sam4l::adc;
use sam4l::usart;

// For panic!()
#[macro_use]
pub mod io;
pub mod version;


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

struct AudioModule {
    console: &'static capsules::console::Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    led: &'static capsules::led::LED<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    adc: &'static capsules::adc::ADC<'static, sam4l::adc::Adc>,
    app_watchdog: &'static signpost_drivers::app_watchdog::AppWatchdog<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    rng: &'static capsules::rng::SimpleRng<'static, sam4l::trng::Trng<'static>>,
    ipc: kernel::ipc::IPC,
}

impl Platform for AudioModule {
    fn with_driver<F, R>(&self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            7 => f(Some(self.adc)),
            8 => f(Some(self.led)),
            13 => f(Some(self.i2c_master_slave)),
            14 => f(Some(self.rng)),

            108 => f(Some(self.app_watchdog)),

            0xff => f(Some(&self.ipc)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA, PB};
    use sam4l::gpio::PeripheralFunction::{A, B};

	// Analog inputs
    PB[02].configure(Some(A)); // MEMS microphone
    PB[03].configure(Some(A)); // External microphone

    // MSGEQ control signals
    PA[07].configure(None);    // spec 2 reset
    PA[08].configure(None);    // spec 2 power
    PA[06].configure(None);    // spec 2 strobe
    PA[05].configure(Some(A));    // spec 2 out
    PA[10].configure(None);    // spec strobe
    PB[00].configure(None);    // spec reset
    PB[01].configure(None);    // spec power
    PA[04].configure(Some(A));    // spec out

    // LEDs
    PB[04].configure(None);    // LEDG2 (LED3)
    PB[05].configure(None);    // LEDR2 (LED4)
    PB[06].configure(None);    // LEDG1 (LED1)
    PB[07].configure(None);    // LEDR1 (LED2)

    // Flash chip
    PA[15].configure(None);    // !FLASH_CS
    PB[11].configure(None);    // !FLASH_RESET
    // using USART3 on 64 pin SAM4L
    PB[08].configure(Some(A)); // FLASH_SCLK
    PB[09].configure(Some(A)); // FLASH_SI
    PB[10].configure(Some(A)); // FLASH_SO

    // Debug lines
    PA[18].configure(None);    // PPS
    // using USART2 on 64 pin SAM4L
    PA[19].configure(None); // Mod out
    PA[20].configure(None); // Mod in
    // using USART0 on 64 pin SAM4L
    PA[12].configure(Some(A)); // DBG_TX
    PA[11].configure(Some(A)); // DBG_RX
    // using TWIMS0 (I2C)
    PA[23].configure(Some(B)); // SDA
	PA[24].configure(Some(B)); // SCL
    // using USBC
	PA[25].configure(Some(A)); // USB-
	PA[26].configure(Some(A)); // USB+
	PB[14].configure(None);    // DBG_GPIO1
	PB[15].configure(None);    // DBG_GPIO2

}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillatorPll, 48000000);
    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

    //
    // UART console
    //
    let console = static_init!(
        capsules::console::Console<usart::USART>,
        capsules::console::Console::new(&usart::USART0,
                     115200,
                     &mut capsules::console::WRITE_BUF,
                     kernel::Container::create()),
        224/8);
    hil::uart::UART::set_client(&usart::USART0, console);

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

    //
    //ADC
    //
    let adc_driver = static_init!(
            capsules::adc::ADC<'static, sam4l::adc::Adc>,
            capsules::adc::ADC::new(&adc::ADC, kernel::Container::create()),
            96/8);
    adc::ADC.set_client(adc_driver);

    // Setup RNG
    let rng = static_init!(
            capsules::rng::SimpleRng<'static, sam4l::trng::Trng>,
            capsules::rng::SimpleRng::new(&sam4l::trng::TRNG, kernel::Container::create()),
            96/8);
    sam4l::trng::TRNG.set_client(rng);

    //
    // App Watchdog
    //
    let app_timeout_alarm = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        24);
    let kernel_timeout_alarm = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        24);
    let app_timeout = static_init!(
        signpost_drivers::app_watchdog::Timeout<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        signpost_drivers::app_watchdog::Timeout::new(app_timeout_alarm, signpost_drivers::app_watchdog::TimeoutMode::App, 1000, cortexm4::scb::reset),
        128/8);
    app_timeout_alarm.set_client(app_timeout);
    let kernel_timeout = static_init!(
        signpost_drivers::app_watchdog::Timeout<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        signpost_drivers::app_watchdog::Timeout::new(kernel_timeout_alarm, signpost_drivers::app_watchdog::TimeoutMode::Kernel, 5000, cortexm4::scb::reset),
        128/8);
    kernel_timeout_alarm.set_client(kernel_timeout);
    let app_watchdog = static_init!(
        signpost_drivers::app_watchdog::AppWatchdog<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        signpost_drivers::app_watchdog::AppWatchdog::new(app_timeout, kernel_timeout),
        64/8);

    //
    // Kernel Watchdog
    //
    let watchdog_alarm = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        24);
    let watchdog = static_init!(
        signpost_drivers::watchdog_kernel::WatchdogKernel<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        signpost_drivers::watchdog_kernel::WatchdogKernel::new(watchdog_alarm, &sam4l::wdt::WDT, 1200),
        128/8);
    watchdog_alarm.set_client(watchdog);

    //
    // Remaining GPIO pins
    //
    let gpio_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 11],
        [&sam4l::gpio::PA[19], //Mod out
         &sam4l::gpio::PA[20], //Mod in
         &sam4l::gpio::PA[18], //PPS
         &sam4l::gpio::PA[10], // spec strobe
         &sam4l::gpio::PB[00], // spec reset
         &sam4l::gpio::PB[01], // spec power
         //&sam4l::gpio::PA[04], // spec out

         &sam4l::gpio::PA[06], // spec 2 strobe
         &sam4l::gpio::PA[07], // spec 2 reset
         &sam4l::gpio::PA[08], // spec 2 power
         //&sam4l::gpio::PA[05], // spec 2 out

         &sam4l::gpio::PA[15], // !FLASH_CS
         &sam4l::gpio::PB[11]], // !FLASH_RESET
        11 * 4
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
        [(&'static sam4l::gpio::GPIOPin, capsules::led::ActivationMode); 6],
          [(&sam4l::gpio::PB[15], capsules::led::ActivationMode::ActiveHigh), //DBG_GPIO1
           (&sam4l::gpio::PB[14], capsules::led::ActivationMode::ActiveHigh), //DBG_GPIO2
           (&sam4l::gpio::PB[06], capsules::led::ActivationMode::ActiveLow),  // LEDG1
           (&sam4l::gpio::PB[07], capsules::led::ActivationMode::ActiveLow),  // LEDR1
           (&sam4l::gpio::PB[04], capsules::led::ActivationMode::ActiveLow),  // LEDG2
           (&sam4l::gpio::PB[05], capsules::led::ActivationMode::ActiveLow)], // LEDR2
           384/8);
    let led = static_init!(
        capsules::led::LED<'static, sam4l::gpio::GPIOPin>,
        capsules::led::LED::new(led_pins),
        64/8);

    // configure initial state for debug LEDs
    sam4l::gpio::PB[15].clear(); // red LED off
    sam4l::gpio::PB[14].set();   // green LED on

    //
    //
    // Actual platform object
    //
    let audio_module = AudioModule {
        console: console,
        gpio: gpio,
        led: led,
        timer: timer,
        i2c_master_slave: i2c_modules,
        adc: adc_driver,
        app_watchdog: app_watchdog,
        rng: rng,
        ipc: kernel::ipc::IPC::new(),
    };

    audio_module.console.initialize();

    //watchdog.start();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    debug!("Running {} Version {} from git {}",
           env!("CARGO_PKG_NAME"),
           env!("CARGO_PKG_VERSION"),
           version::GIT_VERSION,
           );
    kernel::main(&audio_module, &mut chip, load_processes(), &audio_module.ipc);
}
