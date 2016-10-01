
use kernel::common::take_cell::TakeCell;
use core::cell::Cell;
use kernel::hil::gpio;
use kernel::hil::i2c;
use kernel::{AppId, Callback, Driver};

pub static mut BUFFER: [u8; 8] = [0; 8];

pub trait SMBUSIntClient {
    fn interrupt(&self, addr: usize);
    fn done(&self);
}

pub struct SMBUSInterrupt<'a> {
    i2c: &'a i2c::I2CDevice,
    interrupt_pin: Option<&'a gpio::GPIOPin>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static SMBUSIntClient>,
}

impl<'a> SMBUSInterrupt<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice,
        interrupt_pin: Option<&'a gpio::GPIOPin>,
        buffer: &'static mut [u8]) -> SMBUSInterrupt<'a> {
        SMBUSInterrupt {
            i2c: i2c,
            interrupt_pin: interrupt_pin,
            buffer: TakeCell::new(buffer),
            client: TakeCell::empty(),
        }
    } 
    
    pub fn set_client<C: SMBUSIntClient>(&self, client: &'static C) {
        self.client.replace(client);

        self.interrupt_pin.map(|interrupt_pin| {
            interrupt_pin.enable_input(gpio::InputMode::PullUp);
            interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
        });
    }
}

impl<'a> i2c::I2CClient for SMBUSInterrupt<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        //Only ever completes on interrupt
        self.client.map(|client| {
            client.interrupt(buffer[0] as usize);
        });

        self.buffer.replace(buffer);
        self.i2c.disable();
    }
}

impl<'a> gpio::Client for SMBUSInterrupt<'a> {
    fn fired(&self, _: usize) {
            self.buffer.take().map(|buffer| {
            self.i2c.enable();
            self.i2c.read(buffer, 1);
        });
    }
}

pub struct SMBUSIntDriver <'a> {
    smbusint: &'a SMBUSInterrupt<'a>,
    callback: Cell<Option<Callback>>,
}

impl<'a> SMBUSIntDriver <'a> {
    pub fn new(smbusint: &'a SMBUSInterrupt) -> SMBUSIntDriver<'a> {
        SMBUSIntDriver {
            smbusint: smbusint,
            callback: Cell::new(None),
        }
    } 
}

impl<'a> SMBUSIntClient for SMBUSIntDriver<'a> {
    fn interrupt(&self, addr: usize) {
        self.callback.get().map(|mut cb| {
            cb.schedule(0, addr, 0);
        });
    }
    
    fn done(&self) {
        self.callback.get().map(|mut cb| {
            cb.schedule(3, 0, 0);
        });
    }
}

impl<'a> Driver for SMBUSIntDriver<'a> {
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
}

