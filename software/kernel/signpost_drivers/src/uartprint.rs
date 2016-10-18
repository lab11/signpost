use kernel::{AppId, AppSlice, Callback, Shared, Driver};
use kernel::common::take_cell::TakeCell;
use kernel::hil::uart::{self, UART, Client};

pub static mut WRITE_BUF: [u8; 64] = [0; 64];
pub static mut READ_BUF: [u8; 1] = [0];

pub struct UartPrint<'a, U: UART + 'a> {
    uart: &'a U,
    tx_buffer: TakeCell<&'static mut [u8]>,
    _rx_buffer: TakeCell<&'static mut [u8]>,
    app_buffer: TakeCell<AppSlice<Shared, u8>>,
    callback: TakeCell<Callback>,
}

impl<'a, U: UART> UartPrint<'a, U> {
    pub fn new(uart: &'a U,
               tx_buffer: &'static mut [u8],
               rx_buffer: &'static mut [u8])
               -> UartPrint<'a, U> {
        UartPrint {
            uart: uart,
            tx_buffer: TakeCell::new(tx_buffer),
            _rx_buffer: TakeCell::new(rx_buffer),
            app_buffer: TakeCell::empty(),
            callback: TakeCell::empty(),
        }
    }

    pub fn initialize(&self) {
        self.uart.init(uart::UARTParams {
            baud_rate: 115200,
            stop_bits: uart::StopBits::One,
            parity: uart::Parity::None,
            hw_flow_control: false,
        });
    }
}

impl<'a, U: UART> Driver for UartPrint<'a, U> {
    fn allow(&self, _: AppId, allow_num: usize, slice: AppSlice<Shared, u8>) -> isize {
        match allow_num {
            1 => {
                self.app_buffer.replace(slice);
                0
            }
            _ => -1,
        }
    }

    fn subscribe(&self, subscribe_num: usize, callback: Callback) -> isize {
        match subscribe_num {
            1 /* putstr/write_done */ => {

                self.callback.replace(callback);

                self.app_buffer.map(|app_slice| {
                    self.tx_buffer.take().map(|buffer| {
                        for (i, c) in app_slice.as_ref().iter().enumerate() {
                            if buffer.len() <= i {
                                break;
                            }
                            buffer[i] = *c;
                        }
                        self.uart.transmit(buffer, app_slice.len());
                    });
                });
                0
            },
            _ => -1
        }
    }

    fn command(&self, cmd_num: usize, _: usize, _: AppId) -> isize {
        match cmd_num {
            _ => -1
        }
    }
}

impl<'a, U: UART> Client for UartPrint<'a, U> {
    fn transmit_complete(&self, buffer: &'static mut [u8], _error: uart::Error) {
        // Write TX is done, notify appropriate app and start another
        // transaction if pending
        self.tx_buffer.replace(buffer);
        self.callback.map(|mut cb| {
            cb.schedule(0, 0, 0);
        });
    }

    fn receive_complete(&self, _rx_buffer: &'static mut [u8], _rx_len: usize, _error: uart::Error) {
    }
}
