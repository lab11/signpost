use kernel::common::take_cell::TakeCell;
use core::cell::Cell;
use kernel::hil::i2c;
use kernel::{AppId, Callback, Driver};
use kernel::returncode::ReturnCode;

pub static mut BUFFER: [u8; 8] = [0; 8];

// Addresses 0x000 - 0x0FF, 0x180 - 0x1FF can be written as blocks
// Addresses 0x100 - 0x17F must be written by word
#[allow(dead_code)]
#[allow(non_camel_case_types)]
enum Registers {
    Status      = 0x000,
    RepCap      = 0x005, // Reported capacity, LSB = 0.5 mAh
    RepSOC      = 0x006, // Reported capacity, LSB = %/256
    FullCapRep  = 0x035, // Maximum capacity, LSB = 0.5 mAh
    nPackCfg    = 0x1B5, // Pack configuration
    nRSense     = 0x1CF, // Sense resistor
    Batt        = 0x0DA, // Pack voltage, LSB = 1.25mV
    Current     = 0x00A, // Instantaneous current, LSB = 156.25 uA
}

#[derive(Clone,Copy,PartialEq)]
#[allow(dead_code)]
enum State {
    Idle,

    /// Simple read states
    SetupReadStatus,
    ReadStatus,
    SetupReadSOC,
    ReadSOC,
    SetupReadCap,
    ReadCap,
    SetupReadVolt,
    ReadVolt,
    SetupReadCurrent,
    ReadCurrent,

    Done,
}

pub trait MAX17205Client {
    fn status(&self, status: u16);
    fn state_of_charge(&self, percent: u16, capacity: u16, full_capacity: u16);
    fn voltage_current(&self, voltage: u16, current: u16);
    fn done(&self);
}

pub struct MAX17205<'a> {
    i2c0: &'a i2c::I2CDevice,
    i2c1: &'a i2c::I2CDevice,
    state: Cell<State>,
    soc: Cell<u16>,
    soc_mah: Cell<u16>,
    //full_mah: Cell<u16>,
    voltage: Cell<u16>,
    //current: Cell<u16>,
    buffer: TakeCell<'static, [u8]>,
    client: Cell<Option<&'static MAX17205Client>>,
}

impl<'a> MAX17205<'a> {
    pub fn new(i2c0: &'a i2c::I2CDevice,
               i2c1: &'a i2c::I2CDevice,
               buffer: &'static mut [u8])
               -> MAX17205<'a> {
        // setup and return struct
        MAX17205 {
            i2c0: i2c0,
            i2c1: i2c1,
            state: Cell::new(State::Idle),
            soc: Cell::new(0),
            soc_mah: Cell::new(0),
            voltage: Cell::new(0),
            buffer: TakeCell::new(buffer),
            client: Cell::new(None),
        }
    }

    pub fn set_client<C: MAX17205Client>(&self, client: &'static C) {
        self.client.set(Some(client));
    }

    fn configure(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c1.enable();

            let packcfg = 0x0A03; //See PackCfg (59) and typical circuit (28)
            // Memory address (always lower byte, but I2C address is different)
            buffer[0] = ((Registers::nPackCfg as u8) & 0xFF) as u8;
            buffer[1] = (packcfg & 0xFF) as u8;
            buffer[2] = ((packcfg >> 8) & 0xFF) as u8;

            self.i2c1.write(buffer, 3);
            self.state.set(State::Done);
        });
    }

    fn setup_read_status(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c0.enable();

            buffer[0] = ((Registers::Status as u8) & 0xFF) as u8;

            self.i2c0.write(buffer, 2);
            self.state.set(State::SetupReadStatus);
        });
    }

    fn setup_read_soc(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c0.enable();

            // Get SOC mAh and percentage
            // Write reqcap address
            buffer[0] = ((Registers::RepSOC as u8) & 0xFF) as u8;
            self.i2c0.write(buffer, 1);
            self.state.set(State::SetupReadSOC);
        });
    }

    fn setup_read_curvolt(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c0.enable();

            // Get current and voltage
            // Write Batt address
            buffer[0] = ((Registers::Batt as u8) & 0xFF) as u8;
            self.i2c0.write(buffer, 1);
            self.state.set(State::SetupReadVolt);
        });
    }
}

impl<'a> i2c::I2CClient for MAX17205<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            State::SetupReadStatus => {
                // Read status
                self.i2c0.read(buffer, 2);
                self.state.set(State::ReadStatus);
            },
            State::ReadStatus => {
                let status = ((buffer[1] as u16) << 8) | (buffer[0] as u16);

                self.client.get().map(|client| {
                    client.status(status);
                });

                self.buffer.replace(buffer);
                self.i2c0.disable();
                self.state.set(State::Idle);
            }
            State::SetupReadSOC => {
                // Write of SOC memory address complete, now issue read
                self.i2c0.read(buffer, 4);
                self.state.set(State::ReadSOC);
            },
            State::ReadSOC => {
                // Read of SOC memory address complete
                self.soc_mah.set(((buffer[1] as u16) << 8) | (buffer[0] as u16));
                self.soc.set(((buffer[3] as u16) << 8) | (buffer[2] as u16));

                self.buffer.replace(buffer);

                // Now issue write of memory address of full capacity
                // Setup read capacity
                self.buffer.take().map(|selfbuf| {
                    // Get SOC mAh and percentage
                    // Write reqcap address
                    selfbuf[0] = ((Registers::FullCapRep as u8) & 0xFF) as u8;
                    self.i2c0.write(selfbuf, 1);

                    self.state.set(State::SetupReadCap);
                });
            },
            State::SetupReadCap => {
                // Now issue read
                self.i2c0.read(buffer, 2);
                self.state.set(State::ReadCap);
            },
            State::ReadCap => {
                let full_mah = ((buffer[1] as u16) << 8) | (buffer[0] as u16);

                self.client.get().map(|client| {
                    client.state_of_charge(self.soc.get(), self.soc_mah.get(), full_mah);
                });

                self.buffer.replace(buffer);
                self.i2c0.disable();
                self.state.set(State::Idle);
            },
            State::SetupReadVolt => {
                // Write of voltage memory address complete, now issue read
                self.i2c0.read(buffer, 2);
                self.state.set(State::ReadVolt);
            },
            State::ReadVolt => {
                // Read of voltage memory address complete
                self.voltage.set(((buffer[1] as u16) << 8) | (buffer[0] as u16));

                self.buffer.replace(buffer);

                // Now issue write of memory address of current
                // Setup read capacity
                self.buffer.take().map(|selfbuf| {
                    selfbuf[0] = ((Registers::Current as u8) & 0xFF) as u8;
                    self.i2c0.write(selfbuf, 1);

                    self.state.set(State::SetupReadCurrent);
                });
            },
            State::SetupReadCurrent => {
                // Now issue read
                self.i2c0.read(buffer, 2);
                self.state.set(State::ReadCurrent);
            },
            State::ReadCurrent => {
                let current = ((buffer[1] as u16) << 8) | (buffer[0] as u16);

                self.client.get().map(|client| {
                    client.voltage_current(self.voltage.get(), current);
                });

                self.buffer.replace(buffer);
                self.i2c0.disable();
                self.state.set(State::Idle);
            },
            State::Done => {
                self.client.get().map(|client| {
                    client.done();
                });

                self.buffer.replace(buffer);
                self.i2c0.disable();
                self.i2c1.disable();
                self.state.set(State::Idle);
            },
            _ => {}
        }
    }
}

/// Default implementation of the MAX17205 driver that provides a Driver
/// interface for providing access to applications.
pub struct MAX17205Driver<'a> {
    max17205: &'a MAX17205<'a>,
    callback: Cell<Option<Callback>>,
}

impl<'a> MAX17205Driver<'a> {
    pub fn new(max: &'a MAX17205) -> MAX17205Driver<'a> {
        MAX17205Driver {
            max17205: max,
            callback: Cell::new(None),
        }
    }
}

impl<'a> MAX17205Client for MAX17205Driver<'a> {
    fn status(&self, status: u16) {
        self.callback.get().map(|mut cb| {
            cb.schedule(0, status as usize, 0);
        });
    }

    fn state_of_charge(&self, percent: u16, capacity: u16, full_capacity: u16) {
        self.callback.get().map(|mut cb| {
            cb.schedule(1, percent as usize, (capacity as usize) << 16 | (full_capacity as usize));
        });
    }

    fn voltage_current(&self, voltage: u16, current: u16) {
        self.callback.get().map(|mut cb| {
            cb.schedule(2, voltage as usize, current as usize)
        });
    }

    fn done(&self) {
        self.callback.get().map(|mut cb| {
            cb.schedule(3, 0, 0);
        });
    }
}

impl<'a> Driver for MAX17205Driver<'a> {
    fn subscribe(&self, subscribe_num: usize, callback: Callback) -> ReturnCode {
        match subscribe_num {
            0 => {
                self.callback.set(Some(callback));
                ReturnCode::SUCCESS
            }

            // default
            _ => ReturnCode::ENOSUPPORT,
        }
    }

#[allow(unused_variables)]
    fn command(&self, command_num: usize, data: usize, _: AppId) -> ReturnCode {
        match command_num {
            0 => {
                self.max17205.setup_read_status();
                ReturnCode::SUCCESS
            }

            // get soc
            1 => {
                self.max17205.setup_read_soc();
                ReturnCode::SUCCESS
            }

            // get voltage & current
            2 => {
                self.max17205.setup_read_curvolt();
                ReturnCode::SUCCESS
            }

            // configure
            3 => {
                self.max17205.configure();
                ReturnCode::SUCCESS
            }

            // default
            _ => ReturnCode::ENOSUPPORT,
        }
    }
}
