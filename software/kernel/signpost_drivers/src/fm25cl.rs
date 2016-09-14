
use common::take_cell::TakeCell;
use core::cell::Cell;
// use hil::gpio;
use hil;
use main::{AppId, Callback, Driver};

pub static mut BUFFER: [u8; 8] = [0; 8];

#[allow(dead_code)]
enum Opcodes {
    WriteEnable = 0x06,
    WriteDisable = 0x04,
    ReadStatusRegister = 0x05,
    WriteStatusRegister = 0x01,
    ReadMemory = 0x03,
    WriteMemory = 0x02,
}

#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    /// Simple read states
    ReadStatus,
    WriteMemory,
    // ReadShutdown,
    ReadMemory(&'static mut [u8]),

    // Done,
}

// pub enum ChipModel {
//     LTC2941 = 0x01,
//     LTC2942 = 0x00,
// }

// pub enum InterruptPinConf {
//     Disabled = 0x00,
//     ChargeCompleteMode = 0x01,
//     AlertMode = 0x02,
// }

// pub enum VBatAlert {
//     Off = 0x00,
//     Threshold2V8 = 0x01,
//     Threshold2V9 = 0x02,
//     Threshold3V0 = 0x03,
// }

pub trait FM25CLClient {
    // fn interrupt(&self);
    // fn status(&self, undervolt_lockout: bool, vbat_alert: bool, charge_alert_low: bool, charge_alert_high: bool, accumulated_charge_overflow: bool, chip: ChipModel);
    // fn charge(&self, charge: u16);
    fn status(&self, status: u8);
    fn read(&self, data: &'static mut [u8]);
    fn done(&self);
}

pub struct FM25CL<'a> {
    spi: &'a hil::spi_master::SpiMaster,
    // interrupt_pin: Option<&'a gpio::GPIOPin>,
    state: Cell<State>,
    txbuffer: TakeCell<&'static mut [u8]>,
    rxbuffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static FM25CLClient>,
}

impl<'a> FM25CL<'a> {
    pub fn new(spi: &'a hil::spi_master::SpiMaster,
               // interrupt_pin: Option<&'a gpio::GPIOPin>,
               txbuffer: &'static mut [u8],
               rxbuffer: &'static mut [u8])
               -> FM25CL<'a> {
        // setup and return struct
        FM25CL {
            spi: spi,
            // interrupt_pin: interrupt_pin,
            state: Cell::new(State::Idle),
            txbuffer: TakeCell::new(txbuffer),
            rxbuffer: TakeCell::new(rxbuffer),
            client: TakeCell::empty(),
        }
    }

    pub fn set_client<C: FM25CLClient>(&self, client: &'static C, ) {
        self.client.replace(client);

        // self.interrupt_pin.map(|interrupt_pin| {
        //     interrupt_pin.enable_input(gpio::InputMode::PullNone);
        //     interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
        // });
    }

    pub fn read_status(&self) {
        self.txbuffer.take().map(|txbuffer| {
            self.rxbuffer.take().map(move |rxbuffer| {
                txbuffer[0] = Opcodes::ReadStatusRegister as u8;

                self.spi.read_write_bytes(Some(txbuffer), Some(rxbuffer), 2);
                self.state.set(State::ReadStatus);
            });
        });
    }

    fn write(&self, address: u16, buffer: &'static mut [u8], len: u16) {
        self.txbuffer.take().map(|txbuffer| {

            txbuffer[0] = Opcodes::WriteEnable as u8;
            txbuffer[1] = Opcodes::WriteMemory as u8;
            txbuffer[2] = (address >> 8) & 0xFF;
            txbuffer[3] = address & 0xFF;

            for i in 0..len {
                txbuffer[i+4] = buffer[i];
            }

            // self.rxbuffer.take().map(|rxbuffer| {

            //     txbuffer[0] = Opcodes::ReadStatusRegister;

            self.spi.read_write_bytes(Some(txbuffer), None, len+4);

                // self.i2c.enable();

                // // Address pointer automatically resets to the status register.
                // self.i2c.read(buffer, 1);
            self.state.set(State::WriteMemory);
            // });
        });
    }

    fn read(&self, address: u16, buffer: &'static mut [u8], len: u16) {
        self.txbuffer.take().map(|txbuffer| {
            self.rxbuffer.take().map(move |rxbuffer| {
                txbuffer[0] = Opcodes::ReadMemory as u8;
                txbuffer[1] = (address >> 8) & 0xFF;
                txbuffer[2] = address & 0xFF;

                self.spi.read_write_bytes(Some(txbuffer), Some(rxbuffer), len+3);
                self.state.set(State::ReadMemory(buffer));
            });
        });
    }



    // fn configure(&self, int_pin_conf: InterruptPinConf, prescaler: u8, vbat_alert: VBatAlert) {
    //     self.buffer.take().map(|buffer| {
    //         // Encode settings into byte
    //         let control = ((int_pin_conf as u8) << 1) | (prescaler << 3) | ((vbat_alert as u8) << 6);
    //         // Save in buffer
    //         buffer[1] = control;
    //         // Set address
    //         buffer[0] = Registers::Control as u8;

    //         self.i2c.write(buffer, 2);
    //         self.state.set(State::Done);
    //     });
    // }

    // /// Set the accumulated charge to 0
    // fn reset_charge(&self) {
    //     self.buffer.take().map(|buffer| {
    //         self.i2c.enable();

    //         buffer[0] = Registers::AccumulatedChargeMSB as u8;
    //         buffer[1] = 0;
    //         buffer[2] = 0;

    //         self.i2c.write(buffer, 3);
    //         self.state.set(State::Done);
    //     });
    // }

    // fn set_high_threshold(&self, threshold: u16) {
    //     self.buffer.take().map(|buffer| {
    //         self.i2c.enable();

    //         buffer[0] = Registers::ChargeThresholdHighMSB as u8;
    //         buffer[1] = ((threshold & 0xFF00) >> 8) as u8;
    //         buffer[2] = (threshold & 0xFF) as u8;

    //         self.i2c.write(buffer, 3);
    //         self.state.set(State::Done);
    //     });
    // }

    // fn set_low_threshold(&self, threshold: u16) {
    //     self.buffer.take().map(|buffer| {
    //         self.i2c.enable();

    //         buffer[0] = Registers::ChargeThresholdLowMSB as u8;
    //         buffer[1] = ((threshold & 0xFF00) >> 8) as u8;
    //         buffer[2] = (threshold & 0xFF) as u8;

    //         self.i2c.write(buffer, 3);
    //         self.state.set(State::Done);
    //     });
    // }

    // /// Get the cumulative charge as measured by the LTC2941.
    // fn get_charge(&self) {
    //     self.buffer.take().map(|buffer| {
    //         self.i2c.enable();

    //         // Read all of the first four registers rather than wasting
    //         // time writing an address.
    //         self.i2c.read(buffer, 4);
    //         self.state.set(State::ReadCharge);
    //     });
    // }

    // /// Put the LTC2941 in a low power state.
    // fn shutdown(&self) {
    //     self.buffer.take().map(|buffer| {
    //         self.i2c.enable();

    //         // Read both the status and control register rather than
    //         // writing an address.
    //         self.i2c.read(buffer, 2);
    //         self.state.set(State::ReadShutdown);
    //     });
    // }


}

impl<'a> hil::spi_master::SpiCallback for FM25CL<'a> {
    fn read_write_done(&self, write_buffer: Option<&'static mut [u8]>, read_buffer: Option<&'static mut [u8]>, len: usize) {
        match self.state.get() {
            State::ReadStatus => {
                // let status = buffer[0];
                // let uvlock = (status & 0x01) > 0;
                // let vbata = (status & 0x02) > 0;
                // let ca_low = (status & 0x04) > 0;
                // let ca_high = (status & 0x08) > 0;
                // let accover = (status & 0x20) > 0;
                // let chip = match (status & 0x80) >> 7 {
                //     1 => ChipModel::LTC2941,
                //     0 => ChipModel::LTC2942,
                //     _ => ChipModel::LTC2941
                // };
                read_buffer.map(|read_buffer| {
                    self.client.map(|client| {
                        client.status(read_buffer[1]);
                    });
                });
                // self.client.map(|client| {
                //     client.status(uvlock, vbata, ca_low, ca_high, accover, chip);
                // });

                // self.buffer.replace(buffer);
                // self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::WriteMemory => {
                // TODO: Actually calculate charge!!!!!
                // let charge = ((buffer[2] as u16) << 8) | (buffer[3] as u16);
                self.client.map(|client| {
                    client.done();
                });

                // self.buffer.replace(buffer);
                // self.i2c.disable();
                self.state.set(State::Idle);
            },
            State::ReadMemory(buffer) => {

                for i in 0.10 {
                    buffer[i] = read_buffer[3+i];
                }


                // TODO: Actually calculate charge!!!!!
                // let charge = ((buffer[2] as u16) << 8) | (buffer[3] as u16);
                self.client.map(|client| {
                    client.read(buffer);
                });

                // self.buffer.replace(buffer);
                // self.i2c.disable();
                self.state.set(State::Idle);
            },
            // State::ReadShutdown => {
            //     // Set the shutdown pin to 1
            //     buffer[1] |= 0x01;

            //     // Write the control register back but with a in the shutdown
            //     // bit.
            //     buffer[0] = Registers::Control as u8;
            //     self.i2c.write(buffer, 2);
            //     self.state.set(State::Done);
            // },
            // State::Done => {
            //     self.client.map(|client| {
            //         client.done();
            //     });

            //     self.buffer.replace(buffer);
            //     self.i2c.disable();
            //     self.state.set(State::Idle);
            // },
            _ => {}
        }
    }
}

// impl<'a> gpio::Client for LTC2941<'a> {
//     fn fired(&self, _: usize) {
//         self.client.map(|client| {
//             client.interrupt();
//         });
//         // self.buffer.take().map(|buffer| {
//         //     // turn on i2c to send commands
//         //     self.i2c.enable();

//         //     // Just issuing a read to the selector reads its control register.
//         //     self.i2c.read(buffer, 1);
//         //     self.state.set(State::SelectStatus);
//         // });
//     }
// }


// /// Default implementation of the LTC2941 driver that provides a Driver
// /// interface for providing access to applications.
// pub struct LTC2941Driver<'a> {
//     ltc2941: &'a LTC2941<'a>,
//     callback: Cell<Option<Callback>>,
// }

// impl<'a> LTC2941Driver<'a> {
//     pub fn new(ltc: &'a LTC2941) -> LTC2941Driver<'a> {
//         LTC2941Driver {
//             ltc2941: ltc,
//             callback: Cell::new(None),
//         }
//     }
// }

// impl<'a> LTC2941Client for LTC2941Driver<'a> {
//     fn interrupt(&self) {
//         self.callback.get().map(|mut cb| {
//             cb.schedule(0, 0, 0);
//         });
//     }

//     fn status(&self, undervolt_lockout: bool, vbat_alert: bool, charge_alert_low: bool, charge_alert_high: bool, accumulated_charge_overflow: bool, chip: ChipModel) {
//         self.callback.get().map(|mut cb| {
//             let ret = (undervolt_lockout as usize) | ((vbat_alert as usize) << 1) | ((charge_alert_low as usize) << 2) | ((charge_alert_high as usize) << 3) | ((accumulated_charge_overflow as usize) << 4);
//             cb.schedule(1, ret, chip as usize);
//         });
//     }

//     fn charge(&self, charge: u16) {
//         self.callback.get().map(|mut cb| {
//             cb.schedule(2, charge as usize, 0);
//         });
//     }

//     fn done(&self) {
//         self.callback.get().map(|mut cb| {
//             cb.schedule(3, 0, 0);
//         });
//     }
// }

// impl<'a> Driver for LTC2941Driver<'a> {
//     fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
//         match subscribe_num {
//             0 => {
//                 self.callback.set(Some(callback));
//                 0
//             }

//             // default
//             _ => -1,
//         }
//     }

//     fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
//         match command_num {
//             // get status
//             0 => {
//                 self.ltc2941.read_status();
//                 0
//             }

//             // configure
//             1 => {
//                 let int_pin_raw = data & 0x03;
//                 let prescaler = (data >> 2) & 0x07;
//                 let vbat_raw = (data >> 5) & 0x03;
//                 let int_pin_conf = match int_pin_raw {
//                     0 => InterruptPinConf::Disabled,
//                     1 => InterruptPinConf::ChargeCompleteMode,
//                     2 => InterruptPinConf::AlertMode,
//                     _ => InterruptPinConf::Disabled,
//                 };
//                 let vbat_alert = match vbat_raw {
//                     0 => VBatAlert::Off,
//                     1 => VBatAlert::Threshold2V8,
//                     2 => VBatAlert::Threshold2V9,
//                     3 => VBatAlert::Threshold3V0,
//                     _ => VBatAlert::Off,
//                 };

//                 self.ltc2941.configure(int_pin_conf, prescaler as u8, vbat_alert);
//                 0
//             }

//             // reset charge
//             2 => {
//                 self.ltc2941.reset_charge();
//                 0
//             }

//             // set high threshold
//             3 => {
//                 self.ltc2941.set_high_threshold(data as u16);
//                 0
//             }

//             // set low threshold
//             4 => {
//                 self.ltc2941.set_low_threshold(data as u16);
//                 0
//             }

//             // get charge
//             5 => {
//                 self.ltc2941.get_charge();
//                 0
//             }

//             // shutdown
//             6 => {
//                 self.ltc2941.shutdown();
//                 0
//             }

//             // default
//             _ => -1,
//         }
//     }
// }
