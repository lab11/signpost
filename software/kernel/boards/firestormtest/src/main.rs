#![crate_name = "firestormtest"]
#![no_std]
#![no_main]
#![feature(const_fn,lang_items,core_intrinsics)]

extern crate cortexm4;
extern crate capsules;
#[macro_use(static_init)]
extern crate kernel;
extern crate sam4l;

extern crate signpost_drivers;
extern crate signpost_hil;

use capsules::console::{self, Console};
use capsules::nrf51822_serialization::{self, Nrf51822Serialization};
use capsules::timer::TimerDriver;
use capsules::virtual_alarm::{MuxAlarm, VirtualMuxAlarm};
use kernel::hil;
use kernel::hil::Controller;
use kernel::hil::spi::SpiMaster;
use kernel::{Chip, MPU, Platform};
use sam4l::usart;
use sam4l::adc;

// For panic!()
#[macro_use]
pub mod io;


unsafe fn load_processes() -> &'static mut [Option<kernel::process::Process<'static>>] {
    extern "C" {
        /// Beginning of the ROM region containing app images.
        static _sapps: u8;
    }

    const NUM_PROCS: usize = 1;

    #[link_section = ".app_memory"]
    static mut MEMORIES: [[u8; 32768]; NUM_PROCS] = [[0; 32768]; NUM_PROCS];

    static mut processes: [Option<kernel::process::Process<'static>>; NUM_PROCS] = [None];

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

struct SignpostController {
    console: &'static Console<'static, usart::USART>,
    gpio: &'static capsules::gpio::GPIO<'static, sam4l::gpio::GPIOPin>,
    timer: &'static TimerDriver<'static, VirtualMuxAlarm<'static, sam4l::ast::Ast>>,
    smbus_interrupt: &'static signpost_drivers::smbus_interrupt::SMBUSIntDriver<'static>,
    gpio_async: &'static signpost_drivers::gpio_async::GPIOAsync<'static, signpost_drivers::mcp23008::MCP23008<'static>>,
    coulomb_counter_i2c_selector: &'static signpost_drivers::i2c_selector::I2CSelector<'static, signpost_drivers::pca9544a::PCA9544A<'static>>,
    coulomb_counter_generic: &'static signpost_drivers::ltc2941::LTC2941Driver<'static>,
    //fram: &'static signpost_drivers::fm25cl::FM25CLDriver<'static>,
    adc: &'static capsules::adc::ADC<'static, sam4l::adc::Adc>,
    nrf51822: &'static Nrf51822Serialization<'static, usart::USART>,
}

impl Platform for SignpostController {
    fn with_driver<F, R>(&mut self, driver_num: usize, f: F) -> R
        where F: FnOnce(Option<&kernel::Driver>) -> R
    {

        match driver_num {
            0 => f(Some(self.console)),
            1 => f(Some(self.gpio)),
            3 => f(Some(self.timer)),
            5 => f(Some(self.nrf51822)),
            7 => f(Some(self.adc)),
            100 => f(Some(self.gpio_async)),
            101 => f(Some(self.coulomb_counter_i2c_selector)),
            102 => f(Some(self.coulomb_counter_generic)),
            //103 => f(Some(self.fram)),
            104 => f(Some(self.smbus_interrupt)),
            _ => f(None)
        }
    }
}


unsafe fn set_pin_primary_functions() {
    use sam4l::gpio::{PA, PB, PC};
    use sam4l::gpio::PeripheralFunction::{A, B, C, D, E};

    // Configuring pins for RF233
    // SPI
    PC[03].configure(Some(A)); // SPI NPCS0
    PC[02].configure(Some(A)); // SPI NPCS1
    PC[00].configure(Some(A)); // SPI NPCS2
    PC[01].configure(Some(A)); // SPI NPCS3 (RF233)
    PC[06].configure(Some(A)); // SPI CLK
    // PC[04].configure(Some(A)); // SPI MISO
    PC[04].configure(None); // SPI MISO
    PC[05].configure(Some(A)); // SPI MOSI
    // GIRQ line of RF233
    PA[20].enable();
    PA[20].disable_output();
    PA[20].disable_interrupt();
    // PA00 is RCLK
    // PC14 is RSLP
    // PC15 is RRST
    PC[14].enable();
    // PC[14].disable_output();
    PC[14].clear();
    PC[14].enable_output();
    PC[14].clear();


    PC[15].enable();
    // PC[15].disable_output();
    PC[15].set();
    PC[15].enable_output();
    PC[15].set();

    // Right column: Firestorm pin name
    // Left  column: SAM4L peripheral function
    // LI_INT   --  EIC EXTINT2
    PA[04].configure(Some(C));

    // EXTINT1  --  EIC EXTINT1
    PA[06].configure(Some(C));

    // PWM 0    --  GPIO pin
    PA[08].configure(None);

    // PWM 1    --  GPIO pin
    PC[16].configure(None);

    // PWM 2    --  GPIO pin
    PC[17].configure(None);

    // PWM 3    --  GPIO pin
    PC[18].configure(None);

    // AD5      --  ADCIFE AD1
    PA[05].configure(Some(A));

    // AD4      --  ADCIFE AD2
    PA[07].configure(Some(A));

    // AD3      --  ADCIFE AD3
    PB[02].configure(Some(A));

    // AD2      --  ADCIFE AD4
    PB[03].configure(Some(A));

    // AD1      --  ADCIFE AD5
    PB[04].configure(Some(A));

    // AD0      --  ADCIFE AD6
    // PB[05].configure(Some(A));
    PB[05].configure(None);
    PB[05].enable();
    // PC[14].disable_output();
    PB[05].disable_output();
    PB[05].disable_pull_up();
    PB[05].disable_pull_down();




    // BL_SEL   --  USART3 RTS
    PB[06].configure(Some(A));
    //          --  USART3 CTS
    PB[07].configure(Some(A));
    //          --  USART3 CLK
    PB[08].configure(Some(A));
    // PRI_RX   --  USART3 RX
    PB[09].configure(Some(A));
    // PRI_TX   --  USART3 TX
    PB[10].configure(Some(A));
    // U1_CTS   --  USART0 CTS
    PB[11].configure(Some(A));
    // U1_RTS   --  USART0 RTS
    PB[12].configure(Some(A));
    // U1_CLK   --  USART0 CLK
    PB[13].configure(Some(A));
    // U1_RX    --  USART0 RX
    // PB[14].configure(Some(A));
    PB[14].configure(Some(B));



    // U1_TX    --  USART0 TX
    PB[15].configure(Some(A));
    // STORMRTS --  USART2 RTS
    PC[07].configure(Some(B));
    // STORMCTS --  USART2 CTS
    PC[08].configure(Some(E));
    // STORMRX  --  USART2 RX
    PC[11].configure(Some(B));
    // STORMTX  --  USART2 TX
    PC[12].configure(Some(B));
    // STORMCLK --  USART2 CLK
    PA[18].configure(Some(A));

    // ESDA     --  TWIMS1 TWD
    PB[00].configure(Some(A));

    // ESCL     --  TWIMS1 TWCK
    PB[01].configure(Some(A));

    // SDA      --  TWIM2 TWD
    PA[21].configure(Some(E));

    // SCL      --  TWIM2 TWCK
    PA[22].configure(Some(E));

    // EPCLK    --  USBC DM
    PA[25].configure(Some(A));

    // EPDAT    --  USBC DP
    PA[26].configure(Some(A));

    // PCLK     --  PARC PCCK
    PC[21].configure(Some(D));
    // PCEN1    --  PARC PCEN1
    PC[22].configure(Some(D));
    // EPGP     --  PARC PCEN2
    PC[23].configure(Some(D));
    // PCD0     --  PARC PCDATA0
    PC[24].configure(Some(D));
    // PCD1     --  PARC PCDATA1
    PC[25].configure(Some(D));
    // PCD2     --  PARC PCDATA2
    PC[26].configure(Some(D));
    // PCD3     --  PARC PCDATA3
    PC[27].configure(Some(D));
    // PCD4     --  PARC PCDATA4
    // PC[28].configure(Some(D));
    // PC[28].configure(Some(B));  // temp MISO
    PC[28].configure(None);  // temp MISO
    PC[28].enable();
    // PC[14].disable_output();
    PC[28].disable_output();
    PC[28].disable_pull_up();
    PC[28].disable_pull_down();


    // PCD5     --  PARC PCDATA5
    PC[29].configure(Some(D));
    // PCD6     --  PARC PCDATA6
    PC[30].configure(Some(D));
    // PCD7     --  PARC PCDATA7
    PC[31].configure(Some(D));

    // P2       -- GPIO Pin
    PA[16].configure(None);
    // P3       -- GPIO Pin
    PA[12].configure(None);
    // P4       -- GPIO Pin
    PC[09].configure(None);
    // P5       -- GPIO Pin
    PA[10].configure(None);
    // P6       -- GPIO Pin
    PA[11].configure(None);
    // P7       -- GPIO Pin
    PA[19].configure(None);
    // P8       -- GPIO Pin
    PA[13].configure(None);

    // none     -- GPIO Pin
    PA[14].configure(None);

    // ACC_INT2 -- GPIO Pin
    PC[20].configure(None);
    // STORMINT -- GPIO Pin
    PA[17].configure(None);
    // TMP_DRDY -- GPIO Pin
    PA[09].configure(None);
    // ACC_INT1 -- GPIO Pin
    PC[13].configure(None);
    // ENSEN    -- GPIO Pin
    PC[19].configure(None);
    // LED0     -- GPIO Pin
    PC[10].configure(None);
}

/*******************************************************************************
 * Main init function
 ******************************************************************************/

#[no_mangle]
pub unsafe fn reset_handler() {
    sam4l::init();

    sam4l::pm::setup_system_clock(sam4l::pm::SystemClockSource::DfllRc32k, 48000000);

    // Workaround for SB.02 hardware bug
    // TODO(alevy): Get rid of this when we think SB.02 are out of circulation
    sam4l::gpio::PA[14].enable();
    sam4l::gpio::PA[14].set();
    sam4l::gpio::PA[14].enable_output();

    // Source 32Khz and 1Khz clocks from RC23K (SAM4L Datasheet 11.6.8)
    sam4l::bpm::set_ck32source(sam4l::bpm::CK32Source::RC32K);
    let clock_freq = 16000000;

    set_pin_primary_functions();


// // let scif_dfll0conf = ::core::intrinsics::volatile_load(0x400E0828 as *const usize);
// let scif_dfll0 = ::core::intrinsics::volatile_load(0x400E083c as *const usize);

// panic!("before load process {}", scif_dfll0);

    //
    // UART console
    //
    let console = static_init!(
        Console<usart::USART>,
        Console::new(&usart::USART3,
                     115200,
                     &mut console::WRITE_BUF,
                     &mut console::READ_BUF,
                     &mut console::LINE_BUF,
                     kernel::Container::create()),
        416/8);
    usart::USART3.set_uart_client(console);

    // Create the Nrf51822Serialization driver for passing BLE commands
    // over UART to the nRF51822 radio.
    usart::USART2.set_clock_freq(clock_freq);
    let nrf_serialization = static_init!(
        Nrf51822Serialization<usart::USART>,
        Nrf51822Serialization::new(&usart::USART2,
                                   &mut nrf51822_serialization::WRITE_BUF,
                                   &mut nrf51822_serialization::READ_BUF),
        608/8);
    usart::USART2.set_uart_client(nrf_serialization);

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
    let mux_i2c1 = static_init!(capsules::virtual_i2c::MuxI2C<'static>, capsules::virtual_i2c::MuxI2C::new(&sam4l::i2c::I2C1), 20);
    sam4l::i2c::I2C1.set_master_client(mux_i2c1);

    let mux_i2c2 = static_init!(capsules::virtual_i2c::MuxI2C<'static>, capsules::virtual_i2c::MuxI2C::new(&sam4l::i2c::I2C2), 20);
    sam4l::i2c::I2C2.set_master_client(mux_i2c2);

    //
    // SMBUS INTERRUPT
    //

    let smbusint_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x0C),
        32);

    let smbusint = static_init!(
        signpost_drivers::smbus_interrupt::SMBUSInterrupt<'static>,
        // Make sure to replace "None" below with gpio used as SMBUS Alert
        // Some(&sam4l::gpio::PA[16]) for instance
        signpost_drivers::smbus_interrupt::SMBUSInterrupt::new(smbusint_i2c, None, &mut signpost_drivers::smbus_interrupt::BUFFER),
        288/8);

    smbusint_i2c.set_client(smbusint);
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
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x20),
        32);
    let mcp23008_0 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_0_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_0_i2c.set_client(mcp23008_0);

    // Configure the MCP23008_1. Device address 0x21
    let mcp23008_1_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x21),
        32);
    let mcp23008_1 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_1_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_1_i2c.set_client(mcp23008_1);

    // Configure the MCP23008_2. Device address 0x22
    let mcp23008_2_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x22),
        32);
    let mcp23008_2 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_2_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_2_i2c.set_client(mcp23008_2);

    // Configure the MCP23008_5. Device address 0x25
    let mcp23008_5_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x25),
        32);
    let mcp23008_5 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_5_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_5_i2c.set_client(mcp23008_5);

    // Configure the MCP23008_6. Device address 0x26
    let mcp23008_6_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x26),
        32);
    let mcp23008_6 = static_init!(
        signpost_drivers::mcp23008::MCP23008<'static>,
        signpost_drivers::mcp23008::MCP23008::new(mcp23008_6_i2c, None, &mut signpost_drivers::mcp23008::BUFFER),
        320/8);
    mcp23008_6_i2c.set_client(mcp23008_6);

    // Configure the MCP23008_7. Device address 0x27
    let mcp23008_7_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x27),
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
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x70),
        32);
    let pca9544a_0 = static_init!(
        signpost_drivers::pca9544a::PCA9544A<'static>,
        signpost_drivers::pca9544a::PCA9544A::new(pca9544a_0_i2c, &mut signpost_drivers::pca9544a::BUFFER),
        256/8);
    pca9544a_0_i2c.set_client(pca9544a_0);

    let pca9544a_1_i2c = static_init!(
        capsules::virtual_i2c::I2CDevice,
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x71),
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
        capsules::virtual_i2c::I2CDevice::new(mux_i2c1, 0x64),
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
    //let mux_spi = static_init!(
    //    capsules::virtual_spi::MuxSPIMaster<'static>,
    //    capsules::virtual_spi::MuxSPIMaster::new(&sam4l::spi::SPI),
    //    128/8);
    //sam4l::spi::SPI.set_client(mux_spi);
    //sam4l::spi::SPI.init();



    //
    // FRAM
    //
    //let k = hil::spi::ChipSelect::Gpio(&sam4l::gpio::PA[25]);
    //let fm25cl_spi = static_init!(
    //    capsules::virtual_spi::SPIMasterDevice,
    //    capsules::virtual_spi::SPIMasterDevice::new(mux_spi, k),
    //    480/8);
    //let fm25cl = static_init!(
    //    signpost_drivers::fm25cl::FM25CL<'static>,
    //    signpost_drivers::fm25cl::FM25CL::new(fm25cl_spi, &mut signpost_drivers::fm25cl::TXBUFFER, &mut signpost_drivers::fm25cl::RXBUFFER),
    //    384/8);
    //fm25cl_spi.set_client(fm25cl);

    // Interface for applications
    //let fm25cl_driver = static_init!(
    //    signpost_drivers::fm25cl::FM25CLDriver<'static>,
    //    signpost_drivers::fm25cl::FM25CLDriver::new(fm25cl, &mut signpost_drivers::fm25cl::KERNEL_TXBUFFER, &mut signpost_drivers::fm25cl::KERNEL_RXBUFFER),
    //    544/8);
    //fm25cl.set_client(fm25cl_driver);

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
        [&'static sam4l::gpio::GPIOPin; 12],
        [&sam4l::gpio::PC[10], // LED_0
         &sam4l::gpio::PA[16], // P2
         &sam4l::gpio::PA[12], // P3
         &sam4l::gpio::PC[9], // P4
         &sam4l::gpio::PA[10], // P5
         &sam4l::gpio::PA[11], // P6
         &sam4l::gpio::PA[19], // P7
         &sam4l::gpio::PA[13], // P8
         &sam4l::gpio::PA[17], /* STORM_INT (nRF51822) */
         &sam4l::gpio::PC[14], /* RSLP (RF233 sleep line) */
         &sam4l::gpio::PC[15], /* RRST (RF233 reset line) */
         &sam4l::gpio::PA[20]], /* RIRQ (RF233 interrupt) */
        12 * 4
    );
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
    let signpost_controller = static_init!(
        SignpostController,
        SignpostController {
            console: console,
            gpio: gpio,
            timer: timer,
            gpio_async: gpio_async,
            coulomb_counter_i2c_selector: i2c_selector,
            coulomb_counter_generic: ltc2941_driver,
            //fram: fm25cl_driver,
            smbus_interrupt: smbusint_driver,
            adc: adc_driver,
            nrf51822: nrf_serialization,
        },
        288/8);

    // Configure USART2 Pins for connection to nRF51822
    // NOTE: the SAM RTS pin is not working for some reason. Our hypothesis is
    //  that it is because RX DMA is not set up. For now, just having it always
    //  enabled works just fine
    sam4l::gpio::PC[07].enable();
    sam4l::gpio::PC[07].enable_output();
    sam4l::gpio::PC[07].clear();

    signpost_controller.console.initialize();
    signpost_controller.nrf51822.initialize();

    let mut chip = sam4l::chip::Sam4l::new();
    chip.mpu().enable_mpu();

// static mut BUFFER: [u8; 100] = [0; 100];
// sam4l::i2c::I2C1.slave_start();
// sam4l::i2c::I2C1.set_slave_address(0x02);
// // sam4l::i2c::I2C1.receive(&mut BUFFER, 100);
// BUFFER[0] = 0x94;
// BUFFER[1] = 0xbe;
// sam4l::i2c::I2C1.answer_read(&mut BUFFER, 2);




    kernel::main(signpost_controller, &mut chip, load_processes());
}
