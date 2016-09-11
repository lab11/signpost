use core::cell::Cell;
use common::take_cell::TakeCell;
use main::{AppId, Callback, Driver};
use hil::i2c;
use hil;
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
    // callback: Cell<Option<Callback>>,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static signpost_hil::gpio_async::Client>,
}

impl<'a> MCP23008<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice, buffer: &'static mut [u8]) -> MCP23008<'a> {
        // setup and return struct
        MCP23008{
            i2c: i2c,
            // callback: Cell::new(None),
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

                // self.callback.get().map(|mut cb|
                //     cb.schedule(1, pin_value as usize, 0)
                // );

                self.client.map(|client| {
                    client.done(pin_value as usize);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            // State::SelectElectronicId2 => {
            //     self.i2c.read(buffer, 6);
            //     self.state.set(State::ReadElectronicId2);
            // },
            // State::ReadElectronicId2 => {
            //     self.buffer.replace(buffer);
            //     self.i2c.disable();
            //     self.state.set(State::Idle);
            // },
            // State::TakeMeasurementInit => {

            //     let interval = (20 as u32) * <A::Frequency>::frequency() / 1000;

            //     let now = self.alarm.now();
            //     let tics = self.alarm.now().wrapping_add(interval);
            //     self.alarm.set_alarm(tics);

            //     // Now wait for timer to expire
            //     self.buffer.replace(buffer);
            //     self.i2c.disable();
            //     self.state.set(State::Idle);
            // },
            // State::ReadRhMeasurement => {
            //     buffer[2] = buffer[0];
            //     buffer[3] = buffer[1];
            //     buffer[0] = Registers::ReadTemperaturePreviousRHMeasurement as u8;
            //     self.i2c.write(buffer, 1);
            //     self.state.set(State::ReadTempMeasurement);
            // },
            // State::ReadTempMeasurement => {
            //     self.i2c.read(buffer, 2);
            //     self.state.set(State::GotMeasurement);
            // },
            // State::GotMeasurement => {

            //     // Temperature in hundredths of degrees centigrade
            //     let temp_raw = (((buffer[0] as u32) << 8) | (buffer[1] as u32)) as u32;
            //     let temp = (((temp_raw * 17572) / 65536) - 4685) as i16;

            //     // Humidity in hundredths of percent
            //     let humidity_raw = (((buffer[2] as u32) << 8) | (buffer[3] as u32)) as u32;
            //     let humidity = (((humidity_raw * 125 * 100) / 65536) - 600) as u16;

            //     self.callback.get().map(|mut cb|
            //         cb.schedule(temp as usize, humidity as usize, 0)
            //     );

            //     self.buffer.replace(buffer);
            //     self.i2c.disable();
            //     self.state.set(State::Idle);
            // },
            State::Done => {
                // General "I just finished something" callback
                // self.callback.get().map(|mut cb|
                //     cb.schedule(0, 0, 0)
                // );

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



// impl<'a> Driver for MCP23008<'a> {
//     fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
//         match subscribe_num {
//             0 => {
//                 // set callback function
//                 self.callback.set(Some(callback));
//                 0
//             },

//             // default
//             _ => -1
//         }
//     }

//     fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
//         match command_num {
//             // enable output
//             0 => {
//                 if data >= 8 {
//                     // Pin number too high
//                     -1
//                 } else {
//                     self.set_direction(data as u8, Direction::Output);
//                     0
//                 }
//             },

//             // set pin
//             1 => {
//                 if data >=8 {
//                     -1
//                 } else {
//                     self.set_pin(data as u8, PinState::High);
//                     0
//                 }
//             },

//             // clear pin
//             2  => {
//                 if data >= 8 {
//                     -1
//                 } else {
//                     self.set_pin(data as u8, PinState::Low);
//                     0
//                 }
//             },

//             // toggle pin
//             3 => {
//                 if data >= 8 {
//                     -1
//                 } else {
//                     self.toggle_pin(data as u8);
//                     0
//                 }
//             },

//             // enable and configure input
//             4 => {
//                 //XXX: this is clunky
//                 // data == ((pin_config << 8) | pin)
//                 // this allows two values to be passed into a command interface
//                 let pin_num = data & 0xFF;
//                 let pin_config = (data >> 8) & 0xFF;
//                 if pin_num >= 8 {
//                     -1
//                 } else {
//                    self.set_direction(data as u8, Direction::Input);
//                    match pin_config {
//                        0 => { // pull up
//                            self.configure_pullup(data as u8, true);
//                            0
//                        },
//                        1 => { // pull down
//                            // No support for this
//                            -1
//                        },
//                        2 => { // no pull up/down
//                            self.configure_pullup(data as u8, false);
//                            0
//                        },
//                        _ => -1
//                    }
//                 }
//             },

//             // read input
//             5 => {
//                 if data >= 8 {
//                     -1
//                 } else {
//                     self.read_pin(data as u8);
//                     0
//                 }
//             },

//             // enable and configure interrupts on pin, also sets pin as input
//             // (no affect or reliance on registered callback)
//             6 => {
//                 // not yet implemented
//                 0
//             },

//             // disable interrupts on pin, also disables pin
//             // (no affect or reliance on registered callback)
//             7 => {
//                 // not yet implemented
//                 // if data >= pins.len() {
//                 //     -1
//                 // } else {
//                 //     pins[data].disable_interrupt();
//                 //     pins[data].disable();
//                 //     0
//                 // }
//                 0
//             },

//             // disable pin
//             8 => {
//                 // ?
//                 // if data >= pins.len() {
//                 //     -1
//                 // } else {
//                 //     pins[data].disable();
//                 //     0
//                 // }
//                 0
//             }

//             // default
//             _ => -1
//         }
//     }
// }








// pub struct GPIOPin<'a> {
//     mcp23008: &'a MCP23008<'a>,
//     pin: u8,
//     callback: Cell<Option<Callback>>,
// }

// impl<'a> GPIOPin<'a> {
//     pub fn new(mcp23008: &'a MCP23008, pin: u8) -> GPIOPin<'a> {
//         // setup and return struct
//         GPIOPin{
//             mcp23008: mcp23008,
//             pin: pin,
//             callback: Cell::new(None),
//         }
//     }
// }

// impl<'a> hil::gpio::BroadInterface for GPIOPin<'a> {

//     fn set_client(&self, client: &'static hil::gpio::Client) {
//         // self.client.replace(client);
//     }
// }

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





// impl<'a, A: alarm::Alarm + 'a> alarm::AlarmClient for SI7021<'a, A> {
//     fn fired(&self) {
//         self.buffer.take().map(|buffer| {
//             // turn on i2c to send commands
//             self.i2c.enable();

//             self.i2c.read(buffer, 2);
//             self.state.set(State::ReadRhMeasurement);
//         });
//     }
// }

// impl<'a, A: alarm::Alarm + 'a> Driver for SI7021<'a, A> {
//     fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
//         match subscribe_num {
//             // Set a callback
//             0 => {
//                 // Set callback function
//                 self.callback.set(Some(callback));
//                 0
//             },
//             // default
//             _ => -1
//         }
//     }

//     fn command(&self, command_num: usize, _: usize, _: AppId) -> isize {
//         match command_num {
//             // Take a pressure measurement
//             0 => {
//                 self.take_measurement();
//                 0
//             },
//             // default
//             _ => -1
//         }

//     }
// }
