use core::cell::Cell;

use kernel::common::take_cell::TakeCell;
use kernel::{AppId, Callback, Driver};
use kernel::hil::i2c;
use kernel::hil::gpio;

// Buffer to use for I2C messages
pub static mut BUFFER : [u8; 4] = [0; 4];

/// Command register defines
const COMMAND_REG: u8 = 0x80;
const CLEAR_INTERRUPT: u8 = 0x40;
const WORD_PROTOCOL: u8 = 0x20;
const BLOCK_PROTOCOL: u8 = 0x10;

/// Control_Reg defines
const POWER_ON: u8 = 0x03;
const POWER_OFF: u8 = 0x00;

/// Timing_Reg defines
const INTEGRATE_TIME_13P7_MS: u8 = 0x00;
const INTEGRATE_TIME_101_MS: u8 = 0x01;
const INTEGRATE_TIME_402_MS: u8 = 0x02;
const HIGH_GAIN_MODE: u8 = 0x10;
const LOW_GAIN_MODE: u8 = 0x00;
const MANUAL_INTEGRATION: u8 = 0x03;
const BEGIN_INTEGRATION: u8 = 0x08;
const STOP_INTEGRATION: u8 = 0x13;

//Interrupt_Control_Reg defines
const INTERRUPT_CONTROL_DISABLED: u8 = 0x00;
const INTERRUPT_CONTROL_LEVEL: u8 = 0x10;
const INTERRUPT_ON_ADC_DONE: u8 = 0x0;



#[allow(dead_code)]
enum Registers {
    Control = 0x00,
    Timing = 0x01,
    ThresholdLowLow = 0x02,
    ThresholdLowHigh = 0x03,
    ThresholdHighLow = 0x04,
    ThresholdHighHigh = 0x05,
    Interrupt = 0x06,
    Id = 0x0a,
    Data0Low = 0x0c,
    Data0High = 0x0d,
    Data1Low = 0x0e,
    Data1High = 0x0f
}

/// States of the I2C protocol with the LPS331AP.
#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    /// Read the WHO_AM_I register. This should return 0xBB.
    SelectId,
    ReadingId,

    /// Process of taking a pressure measurement.
    /// Start with chip powered off
    // TakeMeasurementInit,
    // /// Then clear the current reading (just in case it exists)
    // /// to reset the interrupt line.
    // TakeMeasurementClear,
    // /// Enable a single shot measurement with interrupt when data is ready.
    // TakeMeasurementConfigure,
    TakeMeasurementTurnOn,
    TakeMeasurementClearInterrupts,
    TakeMeasurementConfigMeasurement,
    TakeMeasurementConfigInterrupts,

    Aaa,
    Bbb,

    /// Read the 3 pressure registers.
    ReadMeasurement,
    /// Calculate pressure and call the callback with the value.
    GotMeasurement,

    /// Disable I2C and release buffer
    Done,
}

pub struct TSL2561<'a> {
    i2c: &'a i2c::I2CDevice,
    interrupt_pin: &'a gpio::Pin,
    callback: Cell<Option<Callback>>,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>
}

impl<'a> TSL2561<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice, interrupt_pin: &'a gpio::Pin, buffer: &'static mut [u8]) -> TSL2561<'a> {
        // setup and return struct
        TSL2561{
            i2c: i2c,
            interrupt_pin: interrupt_pin,
            callback: Cell::new(None),
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer)
        }
    }

    pub fn read_id(&self) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::Id as u8 | COMMAND_REG;
            // buffer[0] = Registers::Id as u8;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectId);
        });
    }

    pub fn take_measurement(&self) {

        // AHH need pull up
        // self.interrupt_pin.enable_input(gpio::InputMode::PullUp);
        self.interrupt_pin.make_input();
        self.interrupt_pin.enable_interrupt(0, gpio::InterruptMode::RisingEdge);

        self.buffer.take().map(|buf| {
            // turn on i2c to send commands
            self.i2c.enable();

            buf[0] = Registers::Control as u8 | COMMAND_REG;
            buf[1] = POWER_ON;
            self.i2c.write(buf, 2);
            // self.state.set(State::TakeMeasurementTurnOn);
            self.state.set(State::Aaa);
        });
    }
}

impl<'a> i2c::I2CClient for TSL2561<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            State::SelectId => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadingId);
            },
            State::ReadingId => {
                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::TakeMeasurementTurnOn => {
                buffer[0] = Registers::Control as u8 | CLEAR_INTERRUPT;
                self.i2c.write(buffer, 1);
                // self.state.set(State::TakeMeasurementClearInterrupts);
                self.state.set(State::Aaa);
            },
            State::Aaa => {
                // buffer[0] = Registers::Control as u8 | CLEAR_INTERRUPT;
                // self.i2c.write(buffer, 1);
                // self.state.set(State::Done);
                buffer[0] = Registers::Control as u8 | COMMAND_REG;
                self.i2c.write(buffer, 1);
                self.state.set(State::Bbb);
            },
            State::Bbb => {
                self.i2c.read(buffer, 1);
                // self.state.set(State::Done);
                self.state.set(State::TakeMeasurementClearInterrupts);
            },
            State::TakeMeasurementClearInterrupts => {
                buffer[0] = Registers::Timing as u8 | COMMAND_REG;
                buffer[1] = INTEGRATE_TIME_101_MS | LOW_GAIN_MODE;
                self.i2c.write(buffer, 2);
                self.state.set(State::TakeMeasurementConfigMeasurement);
            },
            State::TakeMeasurementConfigMeasurement => {
                buffer[0] = Registers::Interrupt as u8 | COMMAND_REG;
                buffer[1] = INTERRUPT_CONTROL_LEVEL | INTERRUPT_ON_ADC_DONE;
                self.i2c.write(buffer, 2);
                self.state.set(State::TakeMeasurementConfigInterrupts);
            },
            State::TakeMeasurementConfigInterrupts => {
                buffer[0] = Registers::Control as u8 | CLEAR_INTERRUPT;
                self.i2c.write(buffer, 1);
                self.state.set(State::Done);
                // buffer[0] = Registers::Interrupt as u8 | COMMAND_REG;
                // buffer[1] = 0x30;
                // self.i2c.write(buffer, 2);
                // self.state.set(State::Done);
            },
            State::ReadMeasurement => {
                self.i2c.read(buffer, 3);
                self.state.set(State::GotMeasurement);
            },
            State::GotMeasurement => {
                let pressure = (((buffer[2] as u32) << 16) | ((buffer[1] as u32) << 8) | (buffer[0] as u32)) as u32;

                // Returned as microbars
                let pressure_ubar = (pressure * 1000) / 4096;

                self.callback.get().map(|mut cb|
                    cb.schedule(pressure_ubar as usize, 0, 0)
                );

                // buffer[0] = Registers::CtrlReg1 as u8;
                // buffer[1] = 0;
                self.i2c.write(buffer, 2);
                self.state.set(State::Done);
            },
            State::Done => {
                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            _ => {}
        }
    }
}

impl<'a> gpio::Client for TSL2561<'a> {
    fn fired(&self, _: usize) {
        // self.buffer.take().map(|buf| {
        //     // turn on i2c to send commands
        //     self.i2c.enable();

        //     // select sensor voltage register and read it
        //     // buf[0] = Registers::PressOutXl as u8 | REGISTER_AUTO_INCREMENT;
        //     self.i2c.write(buf, 1);
        //     self.state.set(State::ReadMeasurement);
        // });
    }
}

impl<'a> Driver for TSL2561<'a> {
    fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
        match subscribe_num {
            // Set a callback
            0 => {
                // Set callback function
                self.callback.set(Some(callback));
                0
            },
            // default
            _ => -1
        }
    }

    fn command(&self, command_num: usize, _: usize, _: AppId) -> isize {
        match command_num {
            // Take a pressure measurement
            0 => {
                self.take_measurement();

                0
            },
            // default
            _ => -1
        }

    }
}
