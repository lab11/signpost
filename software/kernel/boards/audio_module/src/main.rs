#![crate_name = "audio_module"]
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


unsafe fn load_processes() -> &'static mut [Option<kernel::process::Process<'static>>] {
    extern "C" {
        /// Beginning of the ROM region containing app images.
        static _sapps: u8;
    }

    const NUM_PROCS: usize = 2;

    // how should the kernel respond when a process faults
    const FAULT_RESPONSE: kernel::process::FaultResponse = kernel::process::FaultResponse::Panic;
    #[link_section = ".app_memory"]
    static mut APP_MEMORY: [u8; 16384] = [0; 16384];

    static mut processes: [Option<kernel::process::Process<'static>>; NUM_PROCS] = [None, None];

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

        processes[i] = process;
        apps_in_flash_ptr = apps_in_flash_ptr.offset(flash_offset as isize);
        app_memory_ptr = app_memory_ptr.offset(memory_offset as isize);
        app_memory_size -= memory_offset;
    }

    &mut processes
}

/*******************************************************************************
 * Setup this platform
 ******************************************************************************/

struct AudioModule {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    adc: &'static capsules::adc::ADC<'static, sam4l::adc::Adc>,
    app_watchdog: &'static signpost_drivers::app_watchdog::AppWatchdog<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
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

	//analog inputs?
    PA[04].configure(Some(A)); //Input one
    PA[05].configure(Some(A)); //Input two

    PA[06].configure(None); //
    PA[07].configure(None); //2 reset
    PA[08].configure(None); //2 power
    PA[09].configure(None); //2 strobe
    PA[10].configure(None); //strobe
    PA[11].configure(None); //reset
    PA[12].configure(None); //power
    PA[13].configure(None); //
    PA[14].configure(None); //G2
    PA[15].configure(None); //R2
    PA[16].configure(None); //G1
	PA[17].configure(None); //R1
    PA[18].configure(None); //PPS
    PA[19].configure(Some(A)); //Mod out
    PA[20].configure(Some(A)); //Mod in
    PA[21].configure(None); //
    PA[22].configure(None); //
    PA[23].configure(Some(B)); //SDA
	PA[24].configure(Some(B)); //SCL
	PA[25].configure(Some(A)); //USB-
	PA[26].configure(Some(A)); //USB+

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
    let i2c_master_slave = static_init!(
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver::new(&sam4l::i2c::I2C0,
            &mut capsules::i2c_master_slave_driver::BUFFER1,
            &mut capsules::i2c_master_slave_driver::BUFFER2,
            &mut capsules::i2c_master_slave_driver::BUFFER3),
        928/8);
    sam4l::i2c::I2C0.set_master_client(i2c_master_slave);
    sam4l::i2c::I2C0.set_slave_client(i2c_master_slave);

	//
	//ADC
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
        [&'static sam4l::gpio::GPIOPin; 17],
        [&sam4l::gpio::PA[06],
         &sam4l::gpio::PA[07],
         &sam4l::gpio::PA[08],
         &sam4l::gpio::PA[09],
         &sam4l::gpio::PA[10],
         &sam4l::gpio::PA[11],
         &sam4l::gpio::PA[12],
         &sam4l::gpio::PA[13],
         &sam4l::gpio::PA[14],
         &sam4l::gpio::PA[15],
         &sam4l::gpio::PA[16],
         &sam4l::gpio::PA[17],
         &sam4l::gpio::PA[18],
         &sam4l::gpio::PA[19],
         &sam4l::gpio::PA[20],
         &sam4l::gpio::PA[21],
         &sam4l::gpio::PA[22]],
        17 * 4
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
    //
    // Actual platform object
    //
    let audio_module = AudioModule {
        console: console,
        gpio: gpio,
        timer: timer,
        i2c_master_slave: i2c_master_slave,
        adc: adc_driver,
        app_watchdog: app_watchdog,
        ipc: kernel::ipc::IPC::new(),
    };

    audio_module.console.initialize();
	//turn on some LEDs
    sam4l::gpio::PA[14].enable();
    sam4l::gpio::PA[14].enable_output();
    sam4l::gpio::PA[14].set();

    sam4l::gpio::PA[15].enable();
    sam4l::gpio::PA[15].enable_output();
    sam4l::gpio::PA[15].set();

    sam4l::gpio::PA[16].enable();
    sam4l::gpio::PA[16].enable_output();
    sam4l::gpio::PA[16].set();

    sam4l::gpio::PA[17].enable();
    sam4l::gpio::PA[17].enable_output();
    sam4l::gpio::PA[17].set();

	//enable the power to the onboard mic
    sam4l::gpio::PA[08].enable();
    sam4l::gpio::PA[08].enable_output();
    sam4l::gpio::PA[08].clear();

	//disable power to the external mic
	sam4l::gpio::PA[12].enable();
	sam4l::gpio::PA[12].enable_output();
	sam4l::gpio::PA[12].set();


    //watchdog.start();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    kernel::main(&audio_module, &mut chip, load_processes(), &audio_module.ipc);
}
