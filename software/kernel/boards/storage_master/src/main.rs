#![crate_name = "storage_master"]
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

struct SignpostStorageMaster {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
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
            13 => f(Some(self.i2c_master_slave)),

            0xff => f(Some(&self.ipc)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA};
    use sam4l::gpio::PeripheralFunction::{A, B};

    //a few gpio signals
    PA[05].configure(None); // Edison_pwrbtn
    PA[06].configure(None); // linux_enable_power
    PA[07].configure(None); // storage led

    //memory spi bus
    PA[9].configure(None); // Storage CS
    PA[10].configure(Some(A)); // MEMORY_SCLK
    PA[11].configure(Some(A)); // MEMORY_MISO
    PA[12].configure(Some(A)); // MEMORY_MOSI

    //SD card spi bus
    PA[13].configure(None); // SD_CS
    PA[14].configure(Some(A)); // SD_SCLK
    PA[15].configure(Some(A)); // SD_MISO
    PA[16].configure(Some(A)); // SD_MOSI
    PA[17].configure(None); // SD_DETECT
    PA[21].configure(None); // SD_ENABLE

    //Edison SPI Bus
    PA[18].configure(None); // EDISON_SCLK
    PA[19].configure(Some(A)); // EDISON_MOSI //console test
    PA[20].configure(Some(A)); // EDISON_MISO // console test
    PA[22].configure(None); // EDISON_CS

    // I2C: Modules
    PA[23].configure(Some(B)); // MODULES_SDA
    PA[24].configure(Some(B)); // MODULES_SCL
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    // Setup clock
    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

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
    let i2c_modules = static_init!(
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
        capsules::i2c_master_slave_driver::I2CMasterSlaveDriver::new(&sam4l::i2c::I2C0,
            &mut capsules::i2c_master_slave_driver::BUFFER1,
            &mut capsules::i2c_master_slave_driver::BUFFER2,
            &mut capsules::i2c_master_slave_driver::BUFFER3),
        928/8);
    sam4l::i2c::I2C0.set_master_client(i2c_modules);
    sam4l::i2c::I2C0.set_slave_client(i2c_modules);

    // Set I2C slave address here, because it is board specific and not app
    // specific. It can be overridden in the app, of course.
    hil::i2c::I2CSlave::set_address(&sam4l::i2c::I2C0, 0x20);

    //
    // Remaining GPIO pins
    //
    let gpio_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 7],
         [&sam4l::gpio::PA[05],  // EDISON_PWRBTN
         &sam4l::gpio::PA[06],  // LINUX_ENABLE_POWER
         &sam4l::gpio::PA[07],  // STORAGE_LED
         &sam4l::gpio::PA[09],  // STORAGE_CS
         &sam4l::gpio::PA[13],  // SD_CS
         &sam4l::gpio::PA[17],  // SD_DETECT
         &sam4l::gpio::PA[21]], // SD_ENABLE
        7 * 4
    );
        // [&sam4l::gpio::PA[25],  // CONTROLLER_LED { => !FRAM_CS }
    let gpio = static_init!(
        capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
        capsules::gpio::GPIO::new(gpio_pins),
        20);
    for pin in gpio_pins.iter() {
        pin.set_client(gpio);
    }


    //
    // Actual platform object
    //
    let signpost_storage_master = SignpostStorageMaster {
        console: console,
        gpio: gpio,
        timer: timer,
        i2c_master_slave: i2c_modules,
        ipc: kernel::ipc::IPC::new(),
    };

    signpost_storage_master.console.initialize();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    kernel::main(&signpost_storage_master, &mut chip, load_processes(), &signpost_storage_master.ipc);
}
