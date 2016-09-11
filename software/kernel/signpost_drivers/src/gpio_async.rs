use core::cell::Cell;
use signpost_hil;
use hil;
// use hil::gpio::{GPIOPin, InputMode, InterruptMode, Client};
use main::{AppId, Callback, Driver};

pub struct GPIOAsync<'a, Port: signpost_hil::gpio_async::GPIOAsyncPort + 'a> {
    ports: &'a [&'a Port],
    callback: Cell<Option<Callback>>,
}

impl<'a, Port: signpost_hil::gpio_async::GPIOAsyncPort> GPIOAsync<'a, Port> {
    pub fn new(ports: &'a [&'a Port]) -> GPIOAsync<'a, Port> {
        GPIOAsync {
            ports: ports,
            callback: Cell::new(None),
        }
    }

    fn configure_input_pin(&self, port: usize, pin: usize, config: usize) -> isize {
        let ports = self.ports.as_ref();
        match config {
            0 => {
                ports[port].enable_input(pin, hil::gpio::InputMode::PullUp)
            }

            1 => {
                ports[port].enable_input(pin, hil::gpio::InputMode::PullDown)
            }

            2 => {
                ports[port].enable_input(pin, hil::gpio::InputMode::PullNone)
            }

            _ => -1,
        }
    }

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

impl<'a, Port: signpost_hil::gpio_async::GPIOAsyncPort> signpost_hil::gpio_async::Client for GPIOAsync<'a, Port> {
    fn fired(&self, pin_num: usize) {
        // // read the value of the pin
        // let pins = self.pins.as_ref();
        // let pin_state = pins[pin_num].read();

        // // schedule callback with the pin number and value
        // if self.callback.get().is_some() {
        //     self.callback.get().unwrap().schedule(pin_num, pin_state as usize, 0);
        // }
    }

    fn done(&self, value: usize) {
        self.callback.get().map(|mut cb|
            cb.schedule(value, 0, 0)
        );
    }
}

impl<'a, Port: signpost_hil::gpio_async::GPIOAsyncPort> Driver for GPIOAsync<'a, Port> {
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
        let port = data & 0xFF;
        let pin = (data >> 8) & 0xFF;
        let ports = self.ports.as_ref();

        match command_num {
            // enable output
            0 => {
                if port >= ports.len() {
                    -1
                } else {
                    ports[port].enable_output(pin)
                }
            }

            // set pin
            1 => {
                if port >= ports.len() {
                    -1
                } else {
                    ports[port].set(pin)
                }
            }

            // clear pin
            2 => {
                if port >= ports.len() {
                    -1
                } else {
                    ports[port].clear(pin)
                }
            }

            // toggle pin
            3 => {
                if port >= ports.len() {
                    -1
                } else {
                    ports[port].toggle(pin)
                }
            }

            // enable and configure input
            4 => {
                // XXX: this is clunky
                // data == ((pin_config << 8) | pin)
                // this allows two values to be passed into a command interface
                let pin_num = pin & 0xFF;
                let pin_config = (pin >> 8) & 0xFF;
                if port >= ports.len() {
                    -1
                } else {
                    self.configure_input_pin(port, pin_num, pin_config)
                }
            }

            // read input
            5 => {
                if port >= ports.len() {
                    -1
                } else {
                    ports[port].read(pin)
                }
            }

            // enable and configure interrupts on pin, also sets pin as input
            // (no affect or reliance on registered callback)
            6 => {
                // // TODO(brghena): this is clunky
                // // data == ((irq_config << 16) | (pin_config << 8) | pin)
                // // this allows three values to be passed into a command interface
                // let pin_num = data & 0xFF;
                // let pin_config = (data >> 8) & 0xFF;
                // let irq_config = (data >> 16) & 0xFF;
                // if pin_num >= ports.len() {
                //     -1
                // } else {
                //     let mut err_code = self.configure_input_pin(pin_num, pin_config);
                //     if err_code == 0 {
                //         err_code = self.configure_interrupt(pin_num, irq_config);
                //     }
                //     err_code
                // }
                0
            }

            // disable interrupts on pin, also disables pin
            // (no affect or reliance on registered callback)
            7 => {
                // if data >= ports.len() {
                //     -1
                // } else {
                //     ports[data].disable_interrupt();
                //     ports[data].disable();
                //     0
                // }
                0
            }

            // disable pin
            8 => {
                // if data >= ports.len() {
                //     -1
                // } else {
                //     ports[data].disable();
                //     0
                // }
                0
            }

            // default
            _ => -1,
        }
    }
}
