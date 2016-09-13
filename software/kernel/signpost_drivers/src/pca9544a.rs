
use common::math::{sqrtf32, get_errno};
use common::take_cell::TakeCell;
use core::cell::Cell;
use hil::gpio::{GPIOPin, InputMode, InterruptMode, Client};
use hil::gpio;
use hil::i2c;
use main::{AppId, Callback, Driver};
use signpost_hil;

pub static mut BUFFER: [u8; 5] = [0; 5];

// error codes for this driver
// const ERR_BAD_VALUE: isize = -2;

// const MAX_SAMPLING_RATE: u8 = 0x0;
// const DEFAULT_SAMPLING_RATE: u8 = 0x02;

// temperature calculation constants
//  From TMP006 User's Guide section 5.1
//  S_0 should be determined from calibration and ranges from 5E-14 to 7E-14
//  We have selected 5E-14 experimentally
// const S_0: f32 = 5E-14;
// const A_1: f32 = 1.75E-3;
// const A_2: f32 = -1.678E-5;
// const T_REF: f32 = 298.15;
// const B_0: f32 = -2.94E-5;
// const B_1: f32 = -5.7E-7;
// const B_2: f32 = 4.63E-9;
// const C_2: f32 = 13.4;
// const K_TO_C: f32 = -273.15;
// const C_TO_K: f32 = 273.15;
// const NV_TO_V: f32 = 1E9;
// const T_DIE_CONVERT: f32 = 0.03125;
// const V_OBJ_CONVERT: f32 = 156.25;

// #[allow(dead_code)]
// enum Registers {
//     SensorVoltage = 0x00,
//     DieTemperature = 0x01,
//     Configuration = 0x02,
//     ManufacturerID = 0xFE,
//     DeviceID = 0xFF,
// }

// type SensorVoltage = i16;


#[derive(Clone,Copy,PartialEq)]
enum State {
    Idle,

    ReadInterrupts,

    Done,
}

// pub enum Channel {
//     NoChannel = 0,
//     Channel0 = 0x04,
//     Channel1 = 0x05,
//     Channel2 = 0x06,
//     Channel3 = 0x07
// }

pub struct PCA9544A<'a> {
    i2c: &'a i2c::I2CDevice,
    interrupt_pin: &'a gpio::GPIOPin,
    // sampling_period: Cell<u8>,
    // repeated_mode: Cell<bool>,
    // callback: Cell<Option<Callback>>,
    state: Cell<State>,
    buffer: TakeCell<&'static mut [u8]>,
    client: TakeCell<&'static signpost_hil::i2c_selector::Client>,
}

impl<'a> PCA9544A<'a> {
    pub fn new(i2c: &'a i2c::I2CDevice,
               interrupt_pin: &'a GPIOPin,
               buffer: &'static mut [u8])
               -> PCA9544A<'a> {
        // setup and return struct
        PCA9544A {
            i2c: i2c,
            interrupt_pin: interrupt_pin,
            // sampling_period: Cell::new(DEFAULT_SAMPLING_RATE),
            // repeated_mode: Cell::new(false),
            // callback: Cell::new(None),
            state: Cell::new(State::Idle),
            buffer: TakeCell::new(buffer),
            client: TakeCell::empty(),
        }
    }

    pub fn set_client<C: signpost_hil::i2c_selector::Client>(&self, client: &'static C, ) {
        self.client.replace(client);
    }

    /// Choose which channel(s) are active. Channels are encoded with a bitwise
    /// mask (0x01 means enable channel 0, 0x0F means enable all channels).
    /// Send 0 to disable all channels.
    fn select_channels(&self, channel_bitmask: u8) {
        self.buffer.take().map(|buffer| {
            self.i2c.enable();

            // Always clear the settings so we get to a known state
            buffer[0] = 0;

            // Iterate the bit array to send the correct channel enables
            let mut index = 1;
            let i = 0;
            for i in 0..4 {
                if channel_bitmask & (0x01 << i) != 0 {
                    // B2 B1 B0 are set starting at 0x04
                    buffer[index] = i + 4;
                    index += 1;
                }
            }

            // let config = 0x7100 | (((sampling_period & 0x7) as u16) << 9);
            // buf[0] = Registers::Configuration as u8;
            // buf[1] = ((config & 0xFF00) >> 8) as u8;
            // buf[2] = (config & 0x00FF) as u8;
            self.i2c.write(buffer, index as u8);
            self.state.set(State::Done);
        });
    }

    // fn enable_sensor(&self, sampling_period: u8) {
    //     // enable and configure TMP006
    //     self.buffer.take().map(|buf| {
    //         // turn on i2c to send commands
    //         self.i2c.enable();

    //         let config = 0x7100 | (((sampling_period & 0x7) as u16) << 9);
    //         buf[0] = Registers::Configuration as u8;
    //         buf[1] = ((config & 0xFF00) >> 8) as u8;
    //         buf[2] = (config & 0x00FF) as u8;
    //         self.i2c.write(buf, 3);
    //         self.protocol_state.set(ProtocolState::Configure);
    //     });
    // }

    // fn disable_sensor(&self, temperature: Option<f32>) {
    //     // disable the TMP006
    //     self.buffer.take().map(|buf| {
    //         // turn on i2c to send commands
    //         self.i2c.enable();

    //         let config = 0x0000;
    //         buf[0] = Registers::Configuration as u8;
    //         buf[1] = ((config & 0xFF00) >> 8) as u8;
    //         buf[2] = (config & 0x00FF) as u8;
    //         self.i2c.write(buf, 3);
    //         self.protocol_state.set(ProtocolState::Deconfigure(temperature));
    //     });
    // }

    fn enable_interrupts(&self) {
        self.interrupt_pin.enable_input(gpio::InputMode::PullNone);
        self.interrupt_pin.enable_interrupt(0, gpio::InterruptMode::FallingEdge);
    }

    fn disable_interrupts(&self) {
        self.interrupt_pin.disable_interrupt();
        self.interrupt_pin.disable();
    }
}

// fn calculate_temperature(sensor_voltage: i16, die_temperature: i16) -> f32 {
//     // do calculation of actual temperature
//     //  Calculations based on TMP006 User's Guide section 5.1
//     let t_die = ((die_temperature >> 2) as f32) * T_DIE_CONVERT + C_TO_K;
//     let t_adj = t_die - T_REF;
//     let s = S_0 * (1.0 + A_1 * t_adj + A_2 * t_adj * t_adj);

//     let v_obj = (sensor_voltage as f32) * V_OBJ_CONVERT / NV_TO_V;
//     let v_os = B_0 + B_1 * t_adj + B_2 * t_adj * t_adj;

//     let v_adj = v_obj - v_os;
//     let f_v_obj = v_adj + C_2 * v_adj * v_adj;

//     let t_kelvin = sqrtf32(sqrtf32(t_die * t_die * t_die * t_die + (f_v_obj / s)));
//     let t_celsius = t_kelvin + K_TO_C;

//     // return data value
//     t_celsius
// }

impl<'a> i2c::I2CClient for PCA9544A<'a> {
    fn command_complete(&self, buffer: &'static mut [u8], _error: i2c::Error) {
        match self.state.get() {
            // State::SelectChannels => {
            //     self.buffer.replace(buffer);
            //     self.enable_interrupts();
            //     self.i2c.disable();
            //     self.protocol_state.set(ProtocolState::Idle);
            // },
            State::ReadInterrupts => {
                let interrupt_bitmask = (buffer[0] >> 4) & 0x0F;

                self.client.map(|client| {
                    client.interrupts(interrupt_bitmask as usize);
                });

                self.buffer.replace(buffer);
                self.i2c.disable();
                self.state.set(State::Idle);
            }
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

impl<'a> gpio::Client for PCA9544A<'a> {
    fn fired(&self, _: usize) {
        self.buffer.take().map(|buffer| {
            // turn on i2c to send commands
            self.i2c.enable();

            // select sensor voltage register and read it
            // buf[0] = Registers::SensorVoltage as u8;
            self.i2c.read(buffer, 1);
            self.state.set(State::ReadInterrupts);
        });
    }
}

impl<'a> signpost_hil::i2c_selector::I2CSelector for PCA9544A<'a> {
    fn select_channels(&self, channels: usize) {
        self.select_channels(channels as u8);
    }

    fn disable_all_channels(&self) {
        self.select_channels(0);
    }
}

// impl<'a> Driver for TMP006<'a> {
//     fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
//         match subscribe_num {
//             // single temperature reading with callback
//             0 => {
//                 // single sample mode
//                 self.repeated_mode.set(false);

//                 // set callback function
//                 self.callback.set(Some(callback));

//                 // enable sensor
//                 //  turn up the sampling rate so we get the sample faster
//                 self.enable_sensor(MAX_SAMPLING_RATE);

//                 0
//             }

//             // periodic temperature reading subscription
//             1 => {
//                 // periodic sampling mode
//                 self.repeated_mode.set(true);

//                 // set callback function
//                 self.callback.set(Some(callback));

//                 // enable temperature sensor
//                 self.enable_sensor(self.sampling_period.get());

//                 0
//             }

//             // default
//             _ => -1,
//         }
//     }

//     fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
//         match command_num {
//             // set period for sensing
//             0 => {
//                 // bounds check on the period
//                 if (data & 0xFFFFFFF8) != 0 {
//                     return ERR_BAD_VALUE;
//                 }

//                 // set period value
//                 self.sampling_period.set((data & 0x7) as u8);

//                 0
//             }

//             // unsubscribe callback
//             1 => {
//                 // clear callback function
//                 self.callback.set(None);

//                 // disable temperature sensor
//                 self.disable_sensor(None);

//                 0
//             }

//             // default
//             _ => -1,
//         }
//     }
// }
