use core::cell::Cell;
use kernel::{AppId, Callback, Driver};
use kernel::hil;
// use kernel::hil::gpio::{Pin, PinCtl, InputMode, InterruptMode, Client};

pub enum ActivationMode {
    ActiveHigh,
    ActiveLow,
}

pub struct LED<'a, G: hil::gpio::Pin + 'a> {
    pins: &'a [&'a G],
    mode: ActivationMode,
}

impl<'a, G: hil::gpio::Pin + hil::gpio::PinCtl> LED<'a, G> {
    pub fn new(pins: &'a [&'a G], mode: ActivationMode) -> LED<'a, G> {
        // Make all pins output and off
        for pin in pins.iter() {
            pin.make_output();
            match mode {
                ActivationMode::ActiveHigh => pin.clear(),
                ActivationMode::ActiveLow => pin.set(),
            }
        }

        LED {
            pins: pins,
            mode: mode,
        }
    }
}

impl<'a, G: hil::gpio::Pin + hil::gpio::PinCtl> Driver for LED<'a, G> {
    fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
        let pins = self.pins.as_ref();
        match command_num {
            // on
            0 => {
                if data >= pins.len() {
                    -1
                } else {
                    match self.mode {
                        ActivationMode::ActiveHigh => pins[data].set(),
                        ActivationMode::ActiveLow => pins[data].clear(),
                    }
                    0
                }
            }

            // off
            1 => {
                if data >= pins.len() {
                    -1
                } else {
                    match self.mode {
                        ActivationMode::ActiveHigh => pins[data].clear(),
                        ActivationMode::ActiveLow => pins[data].set(),
                    }
                    0
                }
            }

            // toggle
            2 => {
                if data >= pins.len() {
                    -1
                } else {
                    pins[data].toggle();
                    0
                }
            }

            // default
            _ => -1,
        }
    }
}
