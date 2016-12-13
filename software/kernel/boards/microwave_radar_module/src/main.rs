#![crate_name = "microwave_radar_module"]
#![no_std]
#![no_main]
#![feature(const_fn,lang_items)]

extern crate cortexm4;
extern crate capsules;
#[macro_use(static_init)]
extern crate kernel;
extern crate sam4l;

extern crate signpost_drivers;
extern crate signpost_hil;

use capsules::console::{self, Console};
use capsules::timer::TimerDriver;
use sam4l::adc;
use capsules::virtual_alarm::{MuxAlarm, VirtualMuxAlarm};
use kernel::hil::Controller;
use kernel::{Chip, MPU, Platform};
use sam4l::usart;
use kernel::hil;

// For panic!()
#[macro_use]
pub mod io;


unsafe fn load_processes() -> &'static mut [Option<kernel::process::Process<'static>>] {
    extern "C" {
        /// Beginning of the ROM region containing app images.
        static _sapps: u8;
    }

    const NUM_PROCS: usize = 2;

    #[link_section = ".app_memory"]
    static mut MEMORIES: [[u8; 8192]; NUM_PROCS] = [[0; 8192]; NUM_PROCS];

    static mut processes: [Option<kernel::process::Process<'static>>; NUM_PROCS] = [None, None];

    let mut addr = &_sapps as *const u8;
    for i in 0..NUM_PROCS {
        // The first member of the LoadInfo header contains the total size of each process image. A
        // sentinel value of 0 (invalid because it's smaller than the header itself) is used to
        // mark the end of the list of processes.
        let total_size = *(addr as *const usize);
        if total_size == 0 {
            break;
        }

        let process = &mut processes[i];
        let memory = &mut MEMORIES[i];
        *process = Some(kernel::process::Process::create(addr, total_size, memory));
        // TODO: panic if loading failed?

        addr = addr.offset(total_size as isize);
    }

    if *(addr as *const usize) != 0 {
        panic!("Exceeded maximum NUM_PROCS.");
    }

    &mut processes
}

/*******************************************************************************
 * Setup this platform
 ******************************************************************************/

struct MicrowaveRadarModule {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    adc: &'static capsules::adc::ADC<'static, sam4l::adc::Adc>,
    app_watchdog: &'static signpost_drivers::app_watchdog::AppWatchdog<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    ipc: kernel::ipc::IPC,
}

impl Platform for MicrowaveRadarModule {
    fn with_driver<F, R>(&self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            7 => f(Some(self.adc)),
            13 => f(Some(self.i2c_master_slave)),

            108 => f(Some(self.app_watchdog)),

            0xff => f(Some(&self.ipc)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA};
    use sam4l::gpio::PeripheralFunction::{A, B};

    PA[04].configure(Some(A)); // amplified analog signal
    PA[05].configure(Some(A)); // MSGEQ7 output. Should be analog
    PA[06].configure(None); // Unused
    PA[07].configure(None); // Unused
    PA[08].configure(None); // PPS
    PA[09].configure(None); // MOD_IN
    PA[10].configure(None); // MOD_OUT
    PA[11].configure(None); // Unused
    PA[12].configure(None); // Unused
    PA[13].configure(None); // Unused
    PA[14].configure(None); // MSGEQ7 strobe
    PA[15].configure(None); // MSGEQ7 reset
    PA[16].configure(None); // Unused
    PA[17].configure(None); // Blink LED
    PA[18].configure(None); // Unused
    PA[19].configure(None); // Unused
    PA[20].configure(None); // Unused
    PA[21].configure(None); // Unused
    PA[22].configure(None); // Unused
    PA[23].configure(Some(B)); // I2C SDA
    PA[24].configure(Some(B)); // I2C SCL
    PA[25].configure(Some(B)); // USART2 RX
    PA[26].configure(Some(B)); // USART2 TX

    // Setup unused pins as inputs
    sam4l::gpio::PA[06].enable();
    sam4l::gpio::PA[06].disable_output();
    sam4l::gpio::PA[07].enable();
    sam4l::gpio::PA[07].disable_output();
    sam4l::gpio::PA[11].enable();
    sam4l::gpio::PA[11].disable_output();
    sam4l::gpio::PA[12].enable();
    sam4l::gpio::PA[12].disable_output();
    sam4l::gpio::PA[13].enable();
    sam4l::gpio::PA[13].disable_output();
    sam4l::gpio::PA[16].enable();
    sam4l::gpio::PA[16].disable_output();
    sam4l::gpio::PA[18].enable();
    sam4l::gpio::PA[18].disable_output();
    sam4l::gpio::PA[19].enable();
    sam4l::gpio::PA[19].disable_output();
    sam4l::gpio::PA[20].enable();
    sam4l::gpio::PA[20].disable_output();
    sam4l::gpio::PA[21].enable();
    sam4l::gpio::PA[21].disable_output();
    sam4l::gpio::PA[22].enable();
    sam4l::gpio::PA[22].disable_output();

    // Configure LEDs to be off
    sam4l::gpio::PA[17].enable();
    sam4l::gpio::PA[17].enable_output();
    sam4l::gpio::PA[17].clear();
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

    //
    // UART console
    //
    let console = static_init!(
        Console<usart::USART>,
        Console::new(&usart::USART2,
                     115200,
                     &mut console::WRITE_BUF,
                     kernel::Container::create()),
        224/8);
    hil::uart::UART::set_client(&usart::USART2, console);

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
    // To Backplane
    let i2c_master_slave = static_init!(
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver::new(&sam4l::i2c::I2C0,
            &mut capsules::i2c_master_slave_driver::BUFFER1,
            &mut capsules::i2c_master_slave_driver::BUFFER2,
            &mut capsules::i2c_master_slave_driver::BUFFER3),
        928/8);
    sam4l::i2c::I2C0.set_master_client(i2c_master_slave);
    sam4l::i2c::I2C0.set_slave_client(i2c_master_slave);

    // Set I2C slave address here, because it is board specific and not app
    // specific. It can be overridden in the app, of course.
    hil::i2c::I2CSlave::set_address(&sam4l::i2c::I2C0, 0x34);

    //
    // Sensors
    //

    //
    // ADC
    //
    let adc_driver = static_init!(
		    capsules::adc::ADC<'static, sam4l::adc::Adc>,
		    capsules::adc::ADC::new(&adc::ADC),
		    160/8);
    adc::ADC.set_client(adc_driver);

    //
    // Remaining GPIO pins
    //
    let gpio_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 6],
        [&sam4l::gpio::PA[08],
         &sam4l::gpio::PA[09],
         &sam4l::gpio::PA[10],
         &sam4l::gpio::PA[14],
         &sam4l::gpio::PA[15],
         &sam4l::gpio::PA[17]],
        6 * 4
    );
    let gpio = static_init!(
        capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
        capsules::gpio::GPIO::new(gpio_pins),
        20);
    for pin in gpio_pins.iter() {
        pin.set_client(gpio);
    }

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
    // Actual platform object
    //
    let microwave_radar_module = MicrowaveRadarModule {
        console: console,
        gpio: gpio,
        timer: timer,
        i2c_master_slave: i2c_master_slave,
        adc: adc_driver,
        app_watchdog: app_watchdog,
        ipc: kernel::ipc::IPC::new(),
    };

    microwave_radar_module.console.initialize();
    watchdog.start();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    kernel::main(&microwave_radar_module, &mut chip, load_processes(), &microwave_radar_module.ipc);
}
