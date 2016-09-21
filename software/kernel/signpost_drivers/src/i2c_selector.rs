/*
 * Supports up to 8 I2C selectors.
 */

use core::cell::Cell;

use kernel::{AppId, Callback, Driver};

use signpost_hil;

#[derive(Clone,Copy,PartialEq)]
enum Operation {
    SetAll = 0,
    DisableAll = 1,
}

pub struct I2CSelector<'a, Selector: signpost_hil::i2c_selector::I2CSelector + 'a> {
    selectors: &'a [&'a Selector],
    callback: Cell<Option<Callback>>,
    operation: Cell<Operation>,
    index: Cell<u8>,      // Which selector we are currently configuring.
    bitmask: Cell<usize>, // What we are trying to set all channels to.
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> I2CSelector<'a, Selector> {
    pub fn new(selectors: &'a [&'a Selector]) -> I2CSelector<'a, Selector> {
        I2CSelector {
            selectors: selectors,
            callback: Cell::new(None),
            operation: Cell::new(Operation::SetAll),
            index: Cell::new(0),
            bitmask: Cell::new(0),
        }
    }

    // Calls the underlying selector driver to set the correct state.
    // Used in the async loop for setting all needed selectors.
    fn set_channels(&self) {
        let selectors = self.selectors.as_ref();
        let index = self.index.get() as usize;
        let bitmask = self.bitmask.get();

        if selectors.len() > index {
            selectors[index].select_channels((bitmask >> (index*4)) & 0x0F);
        }
    }

    // Calls the underlying selector driver to disable all of its I2C channels.
    fn disable_channels(&self) {
        let selectors = self.selectors.as_ref();
        let index = self.index.get() as usize;

        if selectors.len() > index {
            selectors[index].disable_all_channels();
        }
    }
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> signpost_hil::i2c_selector::Client for I2CSelector<'a, Selector> {
    // Identifier is expected to be the index of the selector in the array
    // that is calling this callback.
    fn interrupts(&self, identifier: usize, interrupt_bitmask: usize) {
        let bitmask = interrupt_bitmask << (identifier * 4);

        self.callback.get().map(|mut cb|
            cb.schedule(1, bitmask, 0)
        );

        // // read the value of the pin
        // let pins = self.pins.as_ref();
        // let pin_state = pins[pin_num].read();

        // // schedule callback with the pin number and value
        // if self.callback.get().is_some() {
        //     self.callback.get().unwrap().schedule(pin_num, pin_state as usize, 0);
        // }
    }

    // Called when underlying selector operation completes.
    fn done(&self) {
        let selectors = self.selectors.as_ref();
        // Increment the selector we are operating on now that we finished
        // the last one.
        let index = self.index.get() + 1;

        // Check if we have more to do
        if selectors.len() > index as usize {
            self.index.set(index);

            // Do the right thing based on what command was called.
            match self.operation.get() {
                Operation::SetAll => {
                    self.set_channels();
                }
                Operation::DisableAll => {
                    self.disable_channels();
                }
            }

        } else {
            // Otherwise we notify the application this finished.
            self.callback.get().map(|mut cb|
                cb.schedule(0, self.operation.get() as usize, 0)
            );
        }
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
        match command_num {
            // select channels
            0 => {
                self.operation.set(Operation::SetAll);
                self.index.set(0);
                self.bitmask.set(data);

                self.set_channels();
                0
            }

            // disable all channels
            1 => {
                self.operation.set(Operation::DisableAll);
                self.index.set(0);

                self.disable_channels();
                0
            }

            // default
            _ => -1,
        }
    }
}
