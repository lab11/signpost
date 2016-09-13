
use common::take_cell::TakeCell;
use core::cell::Cell;
use hil::gpio;
use hil::i2c;

// use signpost_hil;

pub static mut BUFFER: [u8; 5] = [0; 5];

#[allow(dead_code)]
enum Registers {
    Status = 0x00,
    Control = 0x01,
    AccumulatedChargeMSB = 0x02,
    AccumulatedChargeLSB = 0x03,
    ChargeThresholdHighMSB = 0x04,
    ChargeThresholdHighLSB = 0x05,
    ChargeThresholdLowMSB = 0x06,
    ChargeThresholdLowLSB = 0x07,
}

#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    /// Simple read to determine which interrupts are set currently
    SelectStatus,
    ReadStatus,

    SelectCharge,
    ReadCharge,

    SelectShutdown,
    ReadShutdown,

    Done,
}

pub enum ChipModel {
    LTC2941 = 0x01,
    LTC2942 = 0x00,
}

pub enum InterruptPinConf {
    Disabled = 0x00,
    ChargeCompleteMode = 0x01,
    AlertMode = 0x02,
}

pub enum VBatAlert {
    Off = 0x00,
    Threshold2V8 = 0x01,
    Threshold2V9 = 0x02,
    Threshold3V0 = 0x03,
}

pub trait LTC2941Client {
    fn interrupt(&self);
    fn status(&self, undervolt_lockout: bool, vbat_alert: bool, charge_alert_low: bool, charge_alert_high: bool, accumulated_charge_overflow: bool, chip: ChipModel);
    fn charge(&self, charge: u16);
    fn done(&self);
}

pub struct LTC2941<'a> {
    i2c: &'a i2c::I2CDevice,
    interrupt_pin: Option<&'a gpio::GPIOPin>,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static LTC2941Client>,
}

impl<'a> LTC2941<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice,
               interrupt_pin: Option<&'a gpio::GPIOPin>,
               buffer: &'static mut [u8])
               -> LTC2941<'a> {
        // setup and return struct
        LTC2941 {
            i2c: i2c,
            interrupt_pin: interrupt_pin,
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer),
            client: TakeCell::empty(),
        }
    }

    pub fn set_client<C: LTC2941Client>(&self, client: &'static C, ) {
        self.client.replace(client);

        self.interrupt_pin.map(|interrupt_pin| {
            interrupt_pin.enable_input(gpio::InputMode::PullNone);
            interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
        });
    }

    pub fn read_status(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            // Read the status register by first writing the address
            buffer[0] = Registers::Status as u8;

            self.i2c.write(buffer, 1);
            self.state.set(State::SelectStatus);
        });
    }

    fn configure(&self, int_pin_conf: InterruptPinConf, prescaler: u8, vbat_alert: VBatAlert) {
        self.buffer.take().map(|buffer| {
            // Encode settings into byte
            let control = ((int_pin_conf as u8) << 1) | (prescaler << 3) | ((vbat_alert as u8) << 6);
            // Save in buffer
            buffer[1] = control;
            // Set address
            buffer[0] = Registers::Control as u8;

            self.i2c.write(buffer, 2);
            self.state.set(State::Done);
        });
    }

    /// Set the accumulated charge to 0
    fn reset_charge(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            buffer[0] = Registers::AccumulatedChargeMSB as u8;
            buffer[1] = 0;
            buffer[2] = 0;

            self.i2c.write(buffer, 3);
            self.state.set(State::Done);
        });
    }

    fn set_high_threshold(&self, threshold: u16) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            buffer[0] = Registers::ChargeThresholdHighMSB as u8;
            buffer[1] = ((threshold & 0xFF00) >> 8) as u8;
            buffer[2] = (threshold & 0xFF) as u8;

            self.i2c.write(buffer, 3);
            self.state.set(State::Done);
        });
    }

    fn set_low_threshold(&self, threshold: u16) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            buffer[0] = Registers::ChargeThresholdLowMSB as u8;
            buffer[1] = ((threshold & 0xFF00) >> 8) as u8;
            buffer[2] = (threshold & 0xFF) as u8;

            self.i2c.write(buffer, 3);
            self.state.set(State::Done);
        });
    }

    fn get_charge(&self) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            buffer[0] = Registers::AccumulatedChargeMSB as u8;

            self.i2c.write(buffer, 1);
            self.state.set(State::SelectCharge);
        });
    }

    fn shutdown(&self) {
        self.buffer.take().map(|buffer| {
            buffer[0] = Registers::Control as u8;

            self.i2c.write(buffer, 1);
            self.state.set(State::SelectShutdown);
        });
    }


}

impl<'a> i2c::I2CClient for LTC2941<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            State::SelectStatus => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadStatus);
            },
            State::ReadStatus => {
                let status = buffer[0];
                let uvlock = (status & 0x01) > 0;
                let vbata = (status & 0x02) > 0;
                let ca_low = (status & 0x04) > 0;
                let ca_high = (status & 0x08) > 0;
                let accover = (status & 0x20) > 0;
                // let chip = ((status & 0x80) >> 7) as ChipModel::from_u8((status & 0x80) >> 7);
                // let chip = ChipModel::from_u8((status & 0x80) >> 7);
                let chip = match (status & 0x80) >> 7 {
                    1 => ChipModel::LTC2941,
                    0 => ChipModel::LTC2942,
                    _ => ChipModel::LTC2941
                };
                self.client.map(|client| {
                    client.status(uvlock, vbata, ca_low, ca_high, accover, chip);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::SelectCharge => {
                self.i2c.read(buffer, 2);
                self.state.set(State::ReadCharge);
            },
            State::ReadCharge => {
                // TODO: Actually calculate charge!!!!!
                let charge = ((buffer[0] as u16) << 8) | (buffer[1] as u16);
                self.client.map(|client| {
                    client.charge(charge);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::SelectShutdown => {
                self.i2c.read(buffer, 1);
                self.state.set(State::ReadShutdown);
            },
            State::ReadShutdown => {
                // Set the shutdown pin to 1
                buffer[0] |= 0x01;
                self.i2c.read(buffer, 1);
                self.state.set(State::Done);
            },
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

impl<'a> gpio::Client for LTC2941<'a> {
    fn fired(&self, _: usize) {
        self.client.map(|client| {
            client.interrupt();
        });
        // self.buffer.take().map(|buffer| {
        //     // turn on i2c to send commands
        //     self.i2c.enable();

        //     // Just issuing a read to the selector reads its control register.
        //     self.i2c.read(buffer, 1);
        //     self.state.set(State::SelectStatus);
        // });
    }
}

// impl<'a> signpost_hil::i2c_selector::I2CSelector for PCA9544A<'a> {
//     fn select_channels(&self, channels: usize) {
//         self.select_channels(channels as u8);
//     }

//     fn disable_all_channels(&self) {
//         self.select_channels(0);
//     }
// }
