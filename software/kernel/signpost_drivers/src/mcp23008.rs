use core::cell::Cell;
use kernel::common::take_cell::TakeCell;
use kernel::{AppId, Callback, Driver};
use kernel::hil::i2c;
use kernel::hil;
use signpost_hil;

// Buffer to use for I2C messages
pub static mut BUFFER : [u8; 4] = [0; 4];

#[allow(dead_code)]
enum Registers {
    IoDir = 0x00,
    IPol = 0x01,
    GpIntEn = 0x02,
    DefVal = 0x03,
    IntCon = 0x04,
    IoCon = 0x05,
    GpPu = 0x06,
    IntF = 0x07,
    IntCap = 0x08,
    Gpio = 0x09,
    OLat = 0x0a,
}

/// States of the I2C protocol with the MCP23008.
#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    SelectIoDir,
    ReadIoDir,
    SelectGpPu,
    ReadGpPu,
    SelectGpio,
    ReadGpio,
    SelectGpioToggle,
    ReadGpioToggle,
    SelectGpioRead,
    ReadGpioRead,

    /// Disable I2C and release buffer
    Done,
}

enum Direction {
    Input = 0x01,
    Output = 0x00,
}

enum PinState {
    High = 0x01,
    Low = 0x00,
}

pub struct MCP23008<'a> {
    i2c: &'a i2c::I2CDevice,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static signpost_hil::gpio_async::Client>,
}

impl<'a> MCP23008<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice, buffer: &'static mut [u8]) -> MCP23008<'a> {
        // setup and return struct
        MCP23008{
            i2c: i2c,
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer),
            client: TakeCell::empty(),
        }
    }

    pub fn set_client<C: signpost_hil::gpio_async::Client>(&self, client: &'static C, ) {
        self.client.replace(client);
    }

    fn set_direction(&self, pin_number: u8, direction: Direction) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::IoDir as u8;
            // Save settings in buffer so they automatically get passed to
            // state machine.
            buffer[1] = pin_number;
            buffer[2] = direction as u8;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectIoDir);
        });
    }

    fn configure_pullup(&self, pin_number: u8, enabled: bool) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::GpPu as u8;
            // Save settings in buffer so they automatically get passed to
            // state machine.
            buffer[1] = pin_number;
            buffer[2] = enabled as u8;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectGpPu);
        });
    }

    fn set_pin(&self, pin_number: u8, value: PinState) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::Gpio as u8;
            // Save settings in buffer so they automatically get passed to
            // state machine.
            buffer[1] = pin_number;
            buffer[2] = value as u8;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectGpio);
        });
    }

    fn toggle_pin(&self, pin_number: u8) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::Gpio as u8;
            // Save settings in buffer so they automatically get passed to
            // state machine.
            buffer[1] = pin_number;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectGpioToggle);
        });
    }

    fn read_pin(&self, pin_number: u8) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            buffer[0] = Registers::Gpio as u8;
            // Save settings in buffer so they automatically get passed to
            // state machine.
            buffer[1] = pin_number;
            self.i2c.write(buffer, 1);
            self.state.set(State::SelectGpioRead);
        });
    }

}

impl<'a> i2c::I2CClient for MCP23008<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            State::SelectIoDir => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadIoDir);
            },
            State::ReadIoDir => {
                let pin_number = buffer[1];
                let direction = buffer[2];
                if direction == Direction::Input as u8 {
                    buffer[1] = buffer[0] | (1 << pin_number);
                } else {
                    buffer[1] = buffer[0] & !(1 << pin_number);
                }
                buffer[0] = Registers::IoDir as u8;
                self.i2c.write(buffer, 2);
                self.state.set(State::Done);
            },
            State::SelectGpPu => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadGpPu);
            },
            State::ReadGpPu => {
                let pin_number = buffer[1];
                let enabled = buffer[2] == 1;
                if enabled  {
                    buffer[1] = buffer[0] | (1 << pin_number);
                } else {
                    buffer[1] = buffer[0] & !(1 << pin_number);
                }
                buffer[0] = Registers::GpPu as u8;
                self.i2c.write(buffer, 2);
                self.state.set(State::Done);
            },
            State::SelectGpio => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadGpio);
            },
            State::ReadGpio => {
                let pin_number = buffer[1];
                let value = buffer[2];
                if value == PinState::High as u8 {
                    buffer[1] = buffer[0] | (1 << pin_number);
                } else {
                    buffer[1] = buffer[0] & !(1 << pin_number);
                }
                buffer[0] = Registers::Gpio as u8;
                self.i2c.write(buffer, 2);
                self.state.set(State::Done);
            },
            State::SelectGpioToggle => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadGpioToggle);
            },
            State::ReadGpioToggle => {
                let pin_number = buffer[1];
                buffer[1] = buffer[0] ^ (1 << pin_number);
                buffer[0] = Registers::Gpio as u8;
                self.i2c.write(buffer, 2);
                self.state.set(State::Done);
            },
            State::SelectGpioRead => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadGpioRead);
            },
            State::ReadGpioRead => {
                let pin_number = buffer[1];
                let pin_value = (buffer[0] >> pin_number) & 0x01;

                self.client.map(|client| {
                    client.done(pin_value as usize);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::Done => {
                self.client.map(|client| {
                    client.done(0);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            _ => {}
        }
    }
}

impl<'a> signpost_hil::gpio_async::GPIOAsyncPort for MCP23008<'a> {
    fn disable(&self, pin: usize) -> isize {
        // ??
        0
    }

    fn enable_output(&self, pin: usize) -> isize {
        if pin > 7 {
            -1
        } else {
            self.set_direction(pin as u8, Direction::Output);
            0
        }
    }

    fn enable_input(&self, pin: usize, mode: hil::gpio::InputMode) -> isize {
        if pin > 7 {
            -1
        } else {
            self.set_direction(pin as u8, Direction::Input);
            match mode {
                hil::gpio::InputMode::PullUp => {
                    self.configure_pullup(pin as u8, true);
                },
                hil::gpio::InputMode::PullDown => {
                    // No support for this
                },
                hil::gpio::InputMode::PullNone => {
                    self.configure_pullup(pin as u8, false);
                },
            }
            0
        }
    }

    fn read(&self, pin: usize) -> isize {
        if pin > 7 {
            -1
        } else {
            self.read_pin(pin as u8);
            0
        }
    }

    fn toggle(&self, pin: usize) -> isize {
        if pin > 7 {
            -1
        } else {
            self.toggle_pin(pin as u8);
            0
        }
    }

    fn set(&self, pin: usize) -> isize {
        if pin > 7 {
            -1
        } else {
            self.set_pin(pin as u8, PinState::High);
            0
        }
    }

    fn clear(&self, pin: usize) -> isize {
        if pin > 7 {
            -1
        } else {
            self.set_pin(pin as u8, PinState::Low);
            0
        }
    }

    fn enable_interrupt(&self, pin: usize, client_data: usize,
                        mode: hil::gpio::InterruptMode) -> isize {
        // not yet implemented
        0
    }

    fn disable_interrupt(&self, pin: usize) -> isize {
        // not yet implemented
        0
    }
}
