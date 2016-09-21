
use kernel::common::take_cell::TakeCell;
use core::cell::Cell;
use kernel::hil::gpio;
use kernel::hil::i2c;

use signpost_hil;

pub static mut BUFFER: [u8; 5] = [0; 5];

#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    /// Simple read to determine which interrupts are set currently
    ReadInterrupts,

    Done,
}


pub struct PCA9544A<'a> {
    i2c: &'a i2c::I2CDevice,
    interrupt_pin: Option<&'a gpio::GPIOPin>,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static signpost_hil::i2c_selector::Client>,
    identifier: Cell<usize>,
}

impl<'a> PCA9544A<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice,
               interrupt_pin: Option<&'a gpio::GPIOPin>,
               buffer: &'static mut [u8])
               -> PCA9544A<'a> {
        // setup and return struct
        PCA9544A {
            i2c: i2c,
            interrupt_pin: interrupt_pin,
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer),
            client: TakeCell::empty(),
            identifier: Cell::new(0),
        }
    }

    pub fn set_client<C: signpost_hil::i2c_selector::Client>(&self, client: &'static C, identifier: usize) {
        self.client.replace(client);
        self.identifier.set(identifier);

        self.interrupt_pin.map(|interrupt_pin| {
            interrupt_pin.enable_input(gpio::InputMode::PullNone);
            interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
        });
    }

    /// Choose which channel(s) are active. Channels are encoded with a bitwise
    /// mask (0x01 means enable channel 0, 0x0F means enable all channels).
    /// Send 0 to disable all channels.
    fn select_channels(&self, channel_bitmask: u8) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            // Always clear the settings so we get to a known state
            buffer[0] = 0;

            // Iterate the bit array to send the correct channel enables
            let mut index = 1;
            for i in 0..4 {
                if channel_bitmask & (0x01 << i) != 0 {
                    // B2 B1 B0 are set starting at 0x04
                    buffer[index] = i + 4;
                    index += 1;
                }
            }

            self.i2c.write(buffer, index as u8);
            self.state.set(State::Done);
        });
    }

    // fn enable_interrupts(&self) {
    //     self.interrupt_pin.enable_input(gpio::InputMode::PullNone);
    //     self.interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
    // }

    // fn disable_interrupts(&self) {
    //     self.interrupt_pin.disable_interrupt();
    //     self.interrupt_pin.disable();
    // }
}

impl<'a> i2c::I2CClient for PCA9544A<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            State::ReadInterrupts => {
                let interrupt_bitmask = (buffer[0] >> 4) & 0x0F;

                self.client.map(|client| {
                    client.interrupts(self.identifier.get(), interrupt_bitmask as usize);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            }
            State::Done => {
                self.client.map(|client| {
                    client.done();
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            _ => {}
        }
    }
}

impl<'a> gpio::Client for PCA9544A<'a> {
    fn fired(&self, _: usize) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            // Just issuing a read to the selector reads its control register.
            self.i2c.read(buffer, 1);
            self.state.set(State::ReadInterrupts);
        });
    }
}

impl<'a> signpost_hil::i2c_selector::I2CSelector for PCA9544A<'a> {
    fn select_channels(&self, channels: usize) {
        self.select_channels(channels as u8);
    }

    fn disable_all_channels(&self) {
        self.select_channels(0);
    }
}
