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
use kernel::{Chip, MPU, Platform};
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

struct SignpostStorageMaster {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
    bonus_timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
    smbus_interrupt: &'static signpost_drivers::smbus_interrupt::SMBUSIntDriver<'static>,
    gpio_async: &'static signpost_drivers::gpio_async::GPIOAsync<'static, signpost_drivers::mcp23008::MCP23008<'static>>,
    coulomb_counter_i2c_selector: &'static signpost_drivers::i2c_selector::I2CSelector<'static, signpost_drivers::pca9544a::PCA9544A<'static>>,
    coulomb_counter_generic: &'static signpost_drivers::ltc2941::LTC2941Driver<'static>,
    fram: &'static signpost_drivers::fm25cl::FM25CLDriver<'static>,
    i2c_master_slave: &'static signpost_drivers::i2c_master_slave_driver::I2CMasterSlaveDriver<'static>,
}

impl Platform for SignpostStorageMaster {
    fn with_driver<F, R>(&mut self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            100 => f(Some(self.gpio_async)),
            101 => f(Some(self.coulomb_counter_i2c_selector)),
            102 => f(Some(self.coulomb_counter_generic)),
            103 => f(Some(self.fram)),
            104 => f(Some(self.smbus_interrupt)),
            105 => f(Some(self.i2c_master_slave)),
            203 => f(Some(self.bonus_timer)),
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


    // PA[08].configure(None); // MOD6_IN

    // use for FRAM !CS
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
    PA[10].configure(None); // MEMORY_SCLK
    PA[11].configure(None); // MEMORY_MISO
    PA[12].configure(None); // MEMORY_MOSI
    //PA[03].configure(None); // !STORAGE_CS //XXX: check that this works
    PA[25].configure(None); // !FRAM_CS/CONTROLLER_LED

    // UART: GPS
    PA[19].configure(None); // GPS_OUT_TX
    PA[20].configure(None); // GPS_IN_RX

    // SMBus: Power / Backplane
    PA[21].configure(None); // SMBDATA
    PA[22].configure(None); // SMBCLK
    PA[26].configure(None); // !SMBALERT

    // I2C: Modules
    PA[23].configure(None); // MODULES_SDA
    PA[24].configure(None); // MODULES_SCL
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    // Setup clock
    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::ExternalOscillator, 16000000);
    // sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::DfllRc32k, 48000000);
    let clock_freq = 16000000;

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);

    set_pin_primary_functions();
}
