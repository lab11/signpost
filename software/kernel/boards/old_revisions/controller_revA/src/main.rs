#![crate_name = "controller"]
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

use signpost_drivers::gps_console::{self, Console};
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

struct SignpostController {
    gps_console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    bonus_timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    smbus_interrupt: &'static signpost_drivers::smbus_interrupt::SMBUSIntDriver<'static>,
    gpio_async: &'static signpost_drivers::gpio_async::GPIOAsync<'static, signpost_drivers::mcp23008::MCP23008<'static>>,
    coulomb_counter_i2c_selector: &'static signpost_drivers::i2c_selector::I2CSelector<'static, signpost_drivers::pca9544a::PCA9544A<'static>>,
    coulomb_counter_generic: &'static signpost_drivers::ltc2941::LTC2941Driver<'static>,
    fram: &'static capsules::fm25cl::FM25CLDriver<'static, capsules::virtual_spi::VirtualSpiMasterDevice<'static, usart::USART>>,
    i2c_master_slave: &'static capsules::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    app_watchdog: &'static signpost_drivers::app_watchdog::AppWatchdog<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast<'static>>>,
    ipc: kernel::ipc::IPC,
}

impl Platform for SignpostController {
    fn with_driver<F, R>(&self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.gps_console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            13 => f(Some(self.i2c_master_slave)),
            14 => f(Some(self.fram)),

            100 => f(Some(self.gpio_async)),
            101 => f(Some(self.coulomb_counter_i2c_selector)),
            102 => f(Some(self.coulomb_counter_generic)),
            104 => f(Some(self.smbus_interrupt)),
            108 => f(Some(self.app_watchdog)),
            203 => f(Some(self.bonus_timer)),

            0xff => f(Some(&self.ipc)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA};
    use sam4l::gpio::PeripheralFunction::{A, B, E};

    // GPIO: signal from modules
    PA[04].configure(None); // MOD0_IN
    PA[05].configure(None); // MOD1_IN
    PA[06].configure(None); // MOD2_IN
    PA[07].configure(None); // MOD5_IN
    PA[08].configure(None); // MOD6_IN
    PA[09].configure(None); // MOD7_IN

    // GPIO: signal to modules
    PA[13].configure(None); // MOD0_OUT
    PA[14].configure(None); // MOD1_OUT
    PA[15].configure(None); // MOD2_OUT
    PA[16].configure(None); // MOD5_OUT
    PA[17].configure(None); // MOD6_OUT
    PA[18].configure(None); // MOD7_OUT
    PA[18].enable();
    PA[18].enable_output();
    PA[18].set();


    // SPI: Storage Master & FRAM
    PA[10].configure(Some(A)); // MEMORY_SCLK
    PA[11].configure(Some(A)); // MEMORY_MISO
    PA[12].configure(Some(A)); // MEMORY_MOSI
    //PA[03].configure(None); // !STORAGE_CS //XXX: check that this works
    PA[25].configure(None); // !FRAM_CS/CONTROLLER_LED

    // UART: GPS
    PA[19].configure(Some(A)); // GPS_OUT_TX
    PA[20].configure(Some(A)); // GPS_IN_RX

    // SMBus: Power / Backplane
    PA[21].configure(Some(E)); // SMBDATA
    PA[22].configure(Some(E)); // SMBCLK
    PA[26].configure(None); // !SMBALERT

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
    // sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);
    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillatorPll, 48000000);

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

    //
    // UART console
    //
    let gps_console = static_init!(
        Console<usart::USART>,
        Console::new(&usart::USART2,
                     9600,
                     &mut gps_console::WRITE_BUF,
                     &mut gps_console::READ_BUF,
                     &mut gps_console::LINE_BUF,
                     kernel::Container::create()),
        416/8);
    hil::uart::UART::set_client(&usart::USART2, gps_console);

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


    let virtual_alarm2 = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        24);
    let bonus_timer = static_init!(
        TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        TimerDriver::new(virtual_alarm2, kernel::Container::create()),
        12);
    virtual_alarm2.set_client(bonus_timer);

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

    let i2c_mux_smbus = static_init!(
        capsules::virtual_i2c::MuxI2C<'static>,
        capsules::virtual_i2c::MuxI2C::new(&sam4l::i2c::I2C2),
        20);
    sam4l::i2c::I2C2.set_master_client(i2c_mux_smbus);

    //
    // SMBUS INTERRUPT
    //

    let smbusint_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x0C),
        32);

    let smbusint = static_init!(
        signpost_drivers::smbus_interrupt::SMBUSInterrupt<'static>,
        // Make sure to replace "None" below with gpio used as SMBUS Alert
        // Some(&sam4l::gpio::PA[16]) for instance
        signpost_drivers::smbus_interrupt::SMBUSInterrupt::new(smbusint_i2c, None, &mut signpost_drivers::smbus_interrupt::BUFFER),
        288/8);

    smbusint_i2c.set_client(smbusint);
    // Make sure to set smbusint as client for chosen gpio for SMBUS Alert
    // &sam4l::gpio::PA[16].set_client(smbusint); for instance

    let smbusint_driver = static_init!(
        signpost_drivers::smbus_interrupt::SMBUSIntDriver<'static>,
        signpost_drivers::smbus_interrupt::SMBUSIntDriver::new(smbusint),
        128/8);
    smbusint.set_client(smbusint_driver);

    //
    // GPIO EXTENDERS
    //

    // Configure the MCP23008_0. Device address 0x20
    let mcp23008_0_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x20),
        32);
    let mcp23008_0 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_0_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_0_i2c.set_client(mcp23008_0);

    // Configure the MCP23008_1. Device address 0x21
    let mcp23008_1_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x21),
        32);
    let mcp23008_1 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_1_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_1_i2c.set_client(mcp23008_1);

    // Configure the MCP23008_2. Device address 0x22
    let mcp23008_2_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x22),
        32);
    let mcp23008_2 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_2_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_2_i2c.set_client(mcp23008_2);

    // Configure the MCP23008_5. Device address 0x25
    let mcp23008_5_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x25),
        32);
    let mcp23008_5 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_5_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_5_i2c.set_client(mcp23008_5);

    // Configure the MCP23008_6. Device address 0x26
    let mcp23008_6_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x26),
        32);
    let mcp23008_6 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_6_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_6_i2c.set_client(mcp23008_6);

    // Configure the MCP23008_7. Device address 0x27
    let mcp23008_7_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x27),
        32);
    let mcp23008_7 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_7_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_7_i2c.set_client(mcp23008_7);

    // Create an array of the GPIO extenders so we can pass them to an
    // administrative layer that provides a single interface to them all.
    let async_gpio_ports = static_init!(
        [&'static signpost_drivers::mcp23008::MCP23008; 6],
        [mcp23008_0, // Port 0
         mcp23008_1, // Port 1
         mcp23008_2, // Port 2
         mcp23008_5, // Port 3
         mcp23008_6, // Port 4
         mcp23008_7],// Port 5
         192/8
    );

    // `gpio_async` is the object that manages all of the extenders
    let gpio_async = static_init!(
        signpost_drivers::gpio_async::GPIOAsync<'static, signpost_drivers::mcp23008::MCP23008<'static>>,
        signpost_drivers::gpio_async::GPIOAsync::new(async_gpio_ports),
        160/8
    );
    for (i, port) in async_gpio_ports.iter().enumerate() {
        port.set_client(gpio_async, i);
    }

    //
    // I2C Selectors.
    //
    let pca9544a_0_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x70),
        32);
    let pca9544a_0 = static_init!(
        signpost_drivers::pca9544a::PCA9544A<'static>,
        signpost_drivers::pca9544a::PCA9544A::new(pca9544a_0_i2c, &mut signpost_drivers::pca9544a::BUFFER),
        256/8);
    pca9544a_0_i2c.set_client(pca9544a_0);

    let pca9544a_1_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x71),
        32);
    let pca9544a_1 = static_init!(
        signpost_drivers::pca9544a::PCA9544A<'static>,
        signpost_drivers::pca9544a::PCA9544A::new(pca9544a_1_i2c, &mut signpost_drivers::pca9544a::BUFFER),
        256/8);
    pca9544a_1_i2c.set_client(pca9544a_1);

    // Create an array of the I2C selectors so we can give them a single interface
    let i2c_selectors = static_init!(
        [&'static signpost_drivers::pca9544a::PCA9544A; 2],
        [pca9544a_0,
         pca9544a_1],
         64/8
    );

    // This provides the common interface to the I2C selectors
    let i2c_selector = static_init!(
        signpost_drivers::i2c_selector::I2CSelector<'static, signpost_drivers::pca9544a::PCA9544A<'static>>,
        signpost_drivers::i2c_selector::I2CSelector::new(i2c_selectors),
        228/8
    );
    for (i, selector) in i2c_selectors.iter().enumerate() {
        selector.set_client(i2c_selector, i);
    }

    //
    // Coulomb counter
    //

    // Setup the driver for the coulomb counter. We only use one because
    // they all share the same address, so one driver can be used for any
    // of them based on which port is selected on the i2c selector.
    let ltc2941_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_smbus, 0x64),
        32);
    let ltc2941 = static_init!(
        signpost_drivers::ltc2941::LTC2941<'static>,
        signpost_drivers::ltc2941::LTC2941::new(ltc2941_i2c, None, &mut signpost_drivers::ltc2941::BUFFER),
        288/8);
    ltc2941_i2c.set_client(ltc2941);

    // Create the object that provides an interface for the coulomb counter
    // for applications.
    let ltc2941_driver = static_init!(
        signpost_drivers::ltc2941::LTC2941Driver<'static>,
        signpost_drivers::ltc2941::LTC2941Driver::new(ltc2941),
        128/8);
    ltc2941.set_client(ltc2941_driver);


    //
    // SPI
    //
    let mux_spi = static_init!(
        capsules::virtual_spi::MuxSpiMaster<'static, usart::USART>,
        capsules::virtual_spi::MuxSpiMaster::new(&sam4l::usart::USART0),
        96/8);
    // sam4l::spi::SPI.set_client(mux_spi);
    // sam4l::spi::SPI.init();
    hil::spi::SpiMaster::set_client(&sam4l::usart::USART0, mux_spi);
    hil::spi::SpiMaster::init(&sam4l::usart::USART0);



    //
    // FRAM
    //
    // let k = hil::spi::ChipSelect::Gpio(&sam4l::gpio::PA[25]);
    let fm25cl_spi = static_init!(
        capsules::virtual_spi::VirtualSpiMasterDevice<'static, usart::USART>,
        // capsules::virtual_spi::VirtualSpiMasterDevice::new(mux_spi, k),
        capsules::virtual_spi::VirtualSpiMasterDevice::new(mux_spi, &sam4l::gpio::PA[25]),
        416/8);
    let fm25cl = static_init!(
        capsules::fm25cl::FM25CL<'static, capsules::virtual_spi::VirtualSpiMasterDevice<'static, usart::USART>>,
        capsules::fm25cl::FM25CL::new(fm25cl_spi, &mut capsules::fm25cl::TXBUFFER, &mut capsules::fm25cl::RXBUFFER),
        352/8);
    fm25cl_spi.set_client(fm25cl);

    // Interface for applications
    let fm25cl_driver = static_init!(
        capsules::fm25cl::FM25CLDriver<'static, capsules::virtual_spi::VirtualSpiMasterDevice<'static, usart::USART>>,
        capsules::fm25cl::FM25CLDriver::new(fm25cl, &mut capsules::fm25cl::KERNEL_TXBUFFER, &mut capsules::fm25cl::KERNEL_RXBUFFER),
        544/8);
    fm25cl.set_client(fm25cl_driver);

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
        [&'static sam4l::gpio::GPIOPin; 13],
         [&sam4l::gpio::PA[04],  // MOD0_IN
         &sam4l::gpio::PA[05],  // MOD1_IN
         &sam4l::gpio::PA[06],  // MOD2_IN
         &sam4l::gpio::PA[07],  // MOD5_IN
         &sam4l::gpio::PA[08],  // MOD6_IN
         &sam4l::gpio::PA[09],  // MOD7_IN
         &sam4l::gpio::PA[13],  // MOD0_OUT
         &sam4l::gpio::PA[14],  // MOD1_OUT
         &sam4l::gpio::PA[15],  // MOD2_OUT
         &sam4l::gpio::PA[16],  // MOD5_OUT
         &sam4l::gpio::PA[17],  // MOD6_OUT
         &sam4l::gpio::PA[18],  // MOD7_OUT
         &sam4l::gpio::PA[26]], // !SMBALERT
        13 * 4
    );
        // [&sam4l::gpio::PA[25],  // CONTROLLER_LED { => !FRAM_CS }
    let gpio = static_init!(
        capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
        capsules::gpio::GPIO::new(gpio_pins),
        20);
    for pin in gpio_pins.iter() {
        pin.set_client(gpio);
    }

    sam4l::gpio::PA[25].enable();
    sam4l::gpio::PA[25].enable_output();
    sam4l::gpio::PA[25].set();


    //
    // Actual platform object
    //
    let signpost_controller = SignpostController {
        gps_console: gps_console,
        gpio: gpio,
        timer: timer,
        bonus_timer: bonus_timer,
        gpio_async: gpio_async,
        coulomb_counter_i2c_selector: i2c_selector,
        coulomb_counter_generic: ltc2941_driver,
        smbus_interrupt: smbusint_driver,
        fram: fm25cl_driver,
        i2c_master_slave: i2c_modules,
        app_watchdog: app_watchdog,
        ipc: kernel::ipc::IPC::new(),
    };

    signpost_controller.gps_console.initialize();
    watchdog.start();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    kernel::main(&signpost_controller, &mut chip, load_processes(), &signpost_controller.ipc);
}
