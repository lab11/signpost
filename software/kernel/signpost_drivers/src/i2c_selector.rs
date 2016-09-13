use core::cell::Cell;
use signpost_hil;
use hil;
// use hil::gpio::{GPIOPin, InputMode, InterruptMode, Client};
use main::{AppId, Callback, Driver};

pub struct I2CSelector<'a, Selector: signpost_hil::i2c_selector::I2CSelector + 'a> {
    selectors: &'a [&'a Selector],
    callback: Cell<Option<Callback>>,
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> I2CSelector<'a, Selector> {
    pub fn new(selectors: &'a [&'a Selector]) -> I2CSelector<'a, Selector> {
        I2CSelector {
            selectors: selectors,
            callback: Cell::new(None),
        }
    }

    // fn configure_input_pin(&self, port: usize, pin: usize, config: usize) -> isize {
    //     let ports = self.ports.as_ref();
    //     match config {
    //         0 => {
    //             ports[port].enable_input(pin, hil::gpio::InputMode::PullUp)
    //         }

    //         1 => {
    //             ports[port].enable_input(pin, hil::gpio::InputMode::PullDown)
    //         }

    //         2 => {
    //             ports[port].enable_input(pin, hil::gpio::InputMode::PullNone)
    //         }

    //         _ => -1,
    //     }
    // }

    // fn configure_interrupt(&self, pin_num: usize, config: usize) -> isize {
    //     let pins = self.pins.as_ref();
    //     match config {
    //         0 => {
    //             pins[pin_num].enable_interrupt(pin_num, InterruptMode::Change);
    //             0
    //         }

    //         1 => {
    //             pins[pin_num].enable_interrupt(pin_num, InterruptMode::RisingEdge);
    //             0
    //         }

    //         2 => {
    //             pins[pin_num].enable_interrupt(pin_num, InterruptMode::FallingEdge);
    //             0
    //         }

    //         _ => -1,
    //     }
    // }
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> signpost_hil::i2c_selector::Client for I2CSelector<'a, Selector> {
    fn interrupts(&self, interrupt_bitmask: usize) {
        // // read the value of the pin
        // let pins = self.pins.as_ref();
        // let pin_state = pins[pin_num].read();

        // // schedule callback with the pin number and value
        // if self.callback.get().is_some() {
        //     self.callback.get().unwrap().schedule(pin_num, pin_state as usize, 0);
        // }
    }

    fn done(&self) {
        self.callback.get().map(|mut cb|
            cb.schedule(0, 0, 0)
        );
    }
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> Driver for I2CSelector<'a, Selector> {
    fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
        match subscribe_num {
            0 => {
                self.callback.set(Some(callback));
                0
            }

            // default
            _ => -1,
        }
    }

    fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
        // let port = data & 0xFF;
        // let pin = (data >> 8) & 0xFF;
        let selectors = self.selectors.as_ref();

        match command_num {
            // select channels
            0 => {
                // HAKJHDFSLKJSLK
                // AHH
                // TODO
                // This needs to be serialized
                for i in 0..selectors.len() {
                    selectors[i].select_channels((data >> (i*4)) & 0x0F);
                }
                // if port >= ports.len() {
                //     -1
                // } else {
                //     ports[port].enable_output(pin)
                // }
                0
            }

            // disable all channels
            1 => {
                // TODO
                // This needs to be serialized
                for i in 0..selectors.len() {
                    selectors[i].disable_all_channels();
                }
                // if port >= ports.len() {
                //     -1
                // } else {
                //     ports[port].set(pin)
                // }
                0
            }


            // default
            _ => -1,
        }
    }
}
