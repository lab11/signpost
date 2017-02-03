use core::cell::Cell;

use kernel::common::take_cell::TakeCell;
use kernel::hil;
use kernel::returncode::ReturnCode;

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
    i2c: &'a hil::i2c::I2CDevice,
    state: Cell<State>,
    buffer: TakeCell<'static, [u8]>,
    _interrupt_pin: Option<&'static hil::gpio::Pin>,
    identifier: Cell<usize>,
    client: Cell<Option<&'static signpost_hil::gpio_async::Client>>,
}

impl<'a> MCP23008<'a> {
    pub fn new(i2c: &'a hil::i2c::I2CDevice, interrupt_pin: Option<&'static hil::gpio::Pin>, buffer: &'static mut [u8]) -> MCP23008<'a> {
        // setup and return struct
        MCP23008{
            i2c: i2c,
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer),
            _interrupt_pin: interrupt_pin,
            identifier: Cell::new(0),
            client: Cell::new(None),
        }
    }

    pub fn set_client<C: signpost_hil::gpio_async::Client>(&self, client: &'static C, identifier: usize) {
        self.client.set(Some(client));
        self.identifier.set(identifier);
    }

    // fn enable_interrupts(&self, edge: hil::gpio::InterruptMode) {
    //     self.interrupt_pin.map(|interrupt_pin| {
    //         // interrupt_pin.enable_input(hil::gpio::InputMode::PullNone);
    //         interrupt_pin.make_input();
    //         // interrupt_pin.set_input_mode(hil::gpio::InputMode::PullNone);
    //         // hil::gpio::PinCtl::set_input_mode(interrupt_pin, hil::gpio::InputMode::PullNone);
    //         interrupt_pin.enable_interrupt(0, edge);
    //     });
    // }

    // fn disable_interrupts(&self) {
    //     self.interrupt_pin.map(|interrupt_pin| {
    //         interrupt_pin.disable_interrupt();
    //         interrupt_pin.disable();
    //     });
    // }

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

impl<'a> hil::i2c::I2CClient for MCP23008<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: hil::i2c::Error) {
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

                self.client.get().map(|client| {
                    client.done(pin_value as usize);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::Done => {
                self.client.get().map(|client| {
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

impl<'a> hil::gpio::Client for MCP23008<'a> {
    fn fired(&self, _: usize) {

        // TODO: This should ask the chip which pins interrupted.
        self.client.get().map(|client| {
            // Put the port number in the lower half of the forwarded identifier.
            client.fired(self.identifier.get() & 0x00FF);
        });
    }
}

impl<'a> signpost_hil::gpio_async::GPIOAsyncPort for MCP23008<'a> {
    fn disable(&self, pin: usize) -> ReturnCode {
        // Best we can do is make this an input.
        self.set_direction(pin as u8, Direction::Input);
        ReturnCode::SUCCESS
    }

    fn enable_output(&self, pin: usize) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
        } else {
            self.set_direction(pin as u8, Direction::Output);
            ReturnCode::SUCCESS
        }
    }

    fn enable_input(&self, pin: usize, mode: hil::gpio::InputMode) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
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
            ReturnCode::SUCCESS
        }
    }

    fn read(&self, pin: usize) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
        } else {
            self.read_pin(pin as u8);
            ReturnCode::SUCCESS
        }
    }

    fn toggle(&self, pin: usize) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
        } else {
            self.toggle_pin(pin as u8);
            ReturnCode::SUCCESS
        }
    }

    fn set(&self, pin: usize) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
        } else {
            self.set_pin(pin as u8, PinState::High);
            ReturnCode::SUCCESS
        }
    }

    fn clear(&self, pin: usize) -> ReturnCode {
        if pin > 7 {
            ReturnCode::EINVAL
        } else {
            self.set_pin(pin as u8, PinState::Low);
            ReturnCode::SUCCESS
        }
    }

    fn enable_interrupt(&self, _pin: usize, _client_data: usize,
                        _mode: hil::gpio::InterruptMode) -> ReturnCode {
        // not yet implemented
        ReturnCode::SUCCESS
    }

    fn disable_interrupt(&self, _pin: usize) -> ReturnCode {
        // not yet implemented
        ReturnCode::SUCCESS
    }
}
