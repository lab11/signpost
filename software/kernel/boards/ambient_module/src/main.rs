#![crate_name = "ambient_module"]
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

// use capsules::console::{self, Console};
use signpost_drivers::uartprint::{self, UartPrint};
use capsules::timer::TimerDriver;
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

struct AmbientModule {
    // console: &'static Console<'static, usart::USART>,
    uartprint: &'static UartPrint<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    led: &'static signpost_drivers::led::LED<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
    i2c_master_slave: &'static signpost_drivers::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
    // lps331ap: &'static signpost_drivers::lps331ap::LPS331AP<'static>,
    lps25hb: &'static signpost_drivers::lps25hb::LPS25HB<'static>,
    si7021: &'static signpost_drivers::si7021::SI7021<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
    isl29035: &'static capsules::isl29035::Isl29035<'static>,
    tsl2561: &'static signpost_drivers::tsl2561::TSL2561<'static>,
}

impl Platform for AmbientModule {
    fn with_driver<F, R>(&mut self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.uartprint)),
            1 => f(Some(self.gpio)),
            10 => f(Some(self.led)),
            3 => f(Some(self.timer)),
            6 => f(Some(self.isl29035)),
            105 => f(Some(self.i2c_master_slave)),
            106 => f(Some(self.lps25hb)),
            107 => f(Some(self.si7021)),
            108 => f(Some(self.tsl2561)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA};
    use sam4l::gpio::PeripheralFunction::{A, B, E};

    PA[04].configure(None); // PIR
    PA[05].configure(None); // LED1
    PA[06].configure(None); // LED2
    PA[07].configure(None); // LED3
    PA[08].configure(None); // LPS35 Pressure Sensor Interrupt
    PA[09].configure(None); // Unused
    PA[10].configure(None); // LPS25HB Pressure Sensor Interrupt
    PA[11].configure(Some(A)); // UART RX
    PA[12].configure(Some(A)); // UART TX
    PA[13].configure(None); // Unused
    PA[14].configure(None); // LPS331AP Pressure Sensor Interrupt 1
    PA[15].configure(None); // LPS331AP Pressure Sensor Interrupt 2
    PA[16].configure(None); // TSL2561 Light Sensor Interrupt
    PA[17].configure(None); // ISL29035 Light Sensor Interrupt
    PA[18].configure(None); // Module Out
    PA[19].configure(None); // PPS
    PA[20].configure(None); // Module In
    PA[21].configure(Some(E)); // Sensor I2C SDA
    PA[22].configure(Some(E)); // Sensor I2C SCL
    PA[23].configure(Some(B)); // Backplane I2C SDA
    PA[24].configure(Some(B)); // Backplane I2C SCL
    PA[25].configure(Some(A)); // USB-
    PA[26].configure(Some(A)); // USB+

    // Setup unused pins as inputs
    sam4l::gpio::PA[09].enable();
    sam4l::gpio::PA[09].disable_output();
    sam4l::gpio::PA[13].enable();
    sam4l::gpio::PA[13].disable_output();

    // // Configure LEDs to be off
    // sam4l::gpio::PA[05].enable();
    // sam4l::gpio::PA[05].enable_output();
    // sam4l::gpio::PA[05].clear();
    // sam4l::gpio::PA[06].enable();
    // sam4l::gpio::PA[06].enable_output();
    // sam4l::gpio::PA[06].clear();
    // sam4l::gpio::PA[07].enable();
    // sam4l::gpio::PA[07].enable_output();
    // sam4l::gpio::PA[07].set();
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);
    // sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::DfllRc32k, 48000000);
    let clock_freq = 16000000;

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();

    //
    // UART console
    //
    // let console = static_init!(
    //     Console<usart::USART>,
    //     Console::new(&usart::USART0,
    //                  &mut console::WRITE_BUF,
    //                  &mut console::READ_BUF,
    //                  kernel::Container::create()),
    //     256/8);
    // usart::USART0.set_uart_client(console);
    usart::USART0.set_clock_freq(clock_freq);
    let uartprint = static_init!(
        UartPrint<usart::USART>,
        UartPrint::new(&usart::USART0,
                     &mut uartprint::WRITE_BUF,
                     &mut uartprint::READ_BUF),
        384/8);
    usart::USART0.set_uart_client(uartprint);

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
        signpost_drivers::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
        signpost_drivers::i2c_master_slave_driver::I2CMasterSlaveDriver::new(&sam4l::i2c::I2C0,
            &mut signpost_drivers::i2c_master_slave_driver::BUFFER1,
            &mut signpost_drivers::i2c_master_slave_driver::BUFFER2,
            &mut signpost_drivers::i2c_master_slave_driver::BUFFER3),
        928/8);
    sam4l::i2c::I2C0.set_master_client(i2c_master_slave);
    sam4l::i2c::I2C0.set_slave_client(i2c_master_slave);

    // Set I2C slave address here, because it is board specific and not app
    // specific. It can be overridden in the app, of course.
    hil::i2c::I2CSlave::set_address(&sam4l::i2c::I2C0, 0x32);

    // Sensors
    let i2c_mux_sensors = static_init!(
        capsules::virtual_i2c::MuxI2C<'static>,
        capsules::virtual_i2c::MuxI2C::new(&sam4l::i2c::I2C2),
        20);
    sam4l::i2c::I2C2.set_master_client(i2c_mux_sensors);

    //
    // Sensors
    //

    // SI7021 Temperature / Humidity
    let si7021_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_sensors, 0x40),
        32);
    let si7021_virtual_alarm = static_init!(
        VirtualMuxAlarm<'static, sam4l::ast::Ast>,
        VirtualMuxAlarm::new(mux_alarm),
        192/8);
    let si7021 = static_init!(
        signpost_drivers::si7021::SI7021<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
        signpost_drivers::si7021::SI7021::new(si7021_i2c,
            si7021_virtual_alarm,
            &mut signpost_drivers::si7021::BUFFER),
        288/8);
    si7021_i2c.set_client(si7021);
    si7021_virtual_alarm.set_client(si7021);

    // // LPS331AP Pressure Sensor
    // let lps331ap_i2c = static_init!(
    //     capsules::virtual_i2c::I2CDevice,
    //     capsules::virtual_i2c::I2CDevice::new(i2c_mux_sensors, 0x5C),
    //     32);
    // let lps331ap = static_init!(
    //     signpost_drivers::lps331ap::LPS331AP<'static>,
    //     signpost_drivers::lps331ap::LPS331AP::new(lps331ap_i2c,
    //         &sam4l::gpio::PA[14],
    //         &mut signpost_drivers::lps331ap::BUFFER),
    //     40);
    // lps331ap_i2c.set_client(lps331ap);
    // sam4l::gpio::PA[14].set_client(lps331ap);

    // LPS25HB Pressure Sensor
    let lps25hb_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_sensors, 0x5C),
        32);
    let lps25hb = static_init!(
        signpost_drivers::lps25hb::LPS25HB<'static>,
        signpost_drivers::lps25hb::LPS25HB::new(lps25hb_i2c,
            &sam4l::gpio::PA[10],
            &mut signpost_drivers::lps25hb::BUFFER),
        40);
    lps25hb_i2c.set_client(lps25hb);
    sam4l::gpio::PA[10].set_client(lps25hb);

    // TSL2561 Light Sensor
    let tsl2561_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_sensors, 0x29),
        32);
    let tsl2561 = static_init!(
        signpost_drivers::tsl2561::TSL2561<'static>,
        signpost_drivers::tsl2561::TSL2561::new(tsl2561_i2c,
            &sam4l::gpio::PA[16],
            &mut signpost_drivers::tsl2561::BUFFER),
        40);
    tsl2561_i2c.set_client(tsl2561);
    sam4l::gpio::PA[16].set_client(tsl2561);

    // Configure the ISL29035, device address 0x44
    let isl29035_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(i2c_mux_sensors, 0x44),
        32);
    let isl29035 = static_init!(
        capsules::isl29035::Isl29035<'static>,
        capsules::isl29035::Isl29035::new(isl29035_i2c,
            &mut capsules::isl29035::BUF),
        36);
    isl29035_i2c.set_client(isl29035);

    //
    // LEDs
    //
    let led_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 3],
        [&sam4l::gpio::PA[05],
         &sam4l::gpio::PA[06],
         &sam4l::gpio::PA[07]],
        3 * 4
    );
    let led = static_init!(
        signpost_drivers::led::LED<'static, sam4l::gpio::GPIOPin>,
        signpost_drivers::led::LED::new(led_pins, signpost_drivers::led::ActivationMode::ActiveLow),
        96/8);

    //
    // Remaining GPIO pins
    //
    let gpio_pins = static_init!(
        [&'static sam4l::gpio::GPIOPin; 3],
        [&sam4l::gpio::PA[18],
         &sam4l::gpio::PA[19],
         &sam4l::gpio::PA[20]],
        3 * 4
    );
    let gpio = static_init!(
        capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
        capsules::gpio::GPIO::new(gpio_pins),
        20);


    //
    // Actual platform object
    //
    let ambient_module = static_init!(
        AmbientModule,
        AmbientModule {
            // console: console,
            uartprint: uartprint,
            gpio: gpio,
            led: led,
            timer: timer,
            i2c_master_slave: i2c_master_slave,
            lps25hb: lps25hb,
            si7021: si7021,
            isl29035: isl29035,
            tsl2561: tsl2561,
        },
        288/8);

    ambient_module.uartprint.initialize();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

    kernel::main(ambient_module, &mut chip, load_processes());
}
