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
    ReadInterrupts = 2,
    ReadSelected = 3,
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

    // Calls the underlying selector driver to read interrupts.
    fn read_interrupts(&self) {
        let selectors = self.selectors.as_ref();
        let index = self.index.get() as usize;

        if selectors.len() > index {
            selectors[index].read_interrupts();
        }
    }

    fn read_selected(&self) {
        let selectors = self.selectors.as_ref();
        let index = self.index.get() as usize;

        if selectors.len() > index {
            selectors[index].read_selected();
        }
    }
}

impl<'a, Selector: signpost_hil::i2c_selector::I2CSelector> signpost_hil::i2c_selector::Client for I2CSelector<'a, Selector> {
    // Identifier is expected to be the index of the selector in the array
    // that is calling this callback.
    //fn interrupts(&self, identifier: usize, interrupt_bitmask: usize) {
    //    let bitmask = interrupt_bitmask << (identifier * 4);

    //    self.callback.get().map(|mut cb|
    //        cb.schedule(1, bitmask, identifier)
    //    );
    //}

    // Called when underlying selector operation completes.
    fn done(&self, interrupt_bitmask: Option<usize>) {
        // if selector operation returns an interrupt bitmask
        if interrupt_bitmask.is_some() {
            self.bitmask.set(interrupt_bitmask.unwrap() << self.index.get() * 4 | self.bitmask.get());
        }

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
                Operation::ReadInterrupts => {
                    self.read_interrupts();
                }
                Operation::ReadSelected => {
                    self.read_selected();
                }
            }

        } else {
            // Otherwise we notify the application this finished.
            self.callback.get().map(|mut cb|
                cb.schedule(0, self.operation.get() as usize, self.bitmask.get())
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

            2 => {
                self.operation.set(Operation::ReadInterrupts);
                self.index.set(0);
                self.bitmask.set(0);

                self.read_interrupts();
                0
            }

            3 => {
                self.operation.set(Operation::ReadSelected);
                self.index.set(0);
                self.bitmask.set(0);

                self.read_selected();
                0
            }

            // default
            _ => -1,
        }
    }
}
