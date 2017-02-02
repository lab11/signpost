use core::cell::Cell;
use kernel::{AppId, AppSlice, Container, Callback, Shared, Driver};
use kernel::process::Error;
use kernel::returncode::ReturnCode;
use kernel::common::take_cell::TakeCell;
use kernel::hil::uart::{self, UARTAdvanced, Client};

pub struct App {
    read_callback: Option<Callback>,
    write_callback: Option<Callback>,
    read_buffer: Option<AppSlice<Shared, u8>>,
    write_buffer: Option<AppSlice<Shared, u8>>,
    write_len: usize,
    pending_write: bool,
    read_idx: usize,
}

impl Default for App {
    fn default() -> App {
        App {
            read_callback: None,
            write_callback: None,
            read_buffer: None,
            write_buffer: None,
            write_len: 0,
            pending_write: false,
            read_idx: 0,
        }
    }
}

pub static mut WRITE_BUF: [u8; 200] = [0; 200];
pub static mut READ_BUF: [u8; 500] = [0; 500];
pub static mut LINE_BUF: [u8; 100] = [0; 100];

pub struct Console<'a, U: UARTAdvanced + 'a> {
    uart: &'a U,
    apps: Container<App>,
    in_progress: TakeCell<AppId>,
    tx_buffer: TakeCell<&'static mut [u8]>,
    rx_buffer: TakeCell<&'static mut [u8]>,
    line_buffer: TakeCell<&'static mut [u8]>,
    line_idx: Cell<usize>,
    line_complete: Cell<bool>,
    receiving: Cell<bool>,
    baud_rate: Cell<u32>,
}

impl<'a, U: UARTAdvanced> Console<'a, U> {
    pub fn new(uart: &'a U,
               baud_rate: u32,
               tx_buffer: &'static mut [u8],
               rx_buffer: &'static mut [u8],
               line_buffer: &'static mut [u8],
               container: Container<App>)
               -> Console<'a, U> {
        Console {
            uart: uart,
            apps: container,
            in_progress: TakeCell::empty(),
            tx_buffer: TakeCell::new(tx_buffer),
            rx_buffer: TakeCell::new(rx_buffer),
            line_buffer: TakeCell::new(line_buffer),
            line_idx: Cell::new(0),
            line_complete: Cell::new(false),
            receiving: Cell::new(false),
            baud_rate: Cell::new(baud_rate),
        }
    }

    pub fn initialize(&self) {
        self.uart.init(uart::UARTParams {
            baud_rate: self.baud_rate.get(),
            stop_bits: uart::StopBits::One,
            parity: uart::Parity::None,
            hw_flow_control: false,
        });
    }
}

impl<'a, U: UARTAdvanced> Driver for Console<'a, U> {
    fn allow(&self, appid: AppId, allow_num: usize, slice: AppSlice<Shared, u8>) -> ReturnCode {
        match allow_num {
            // Allow a read buffer
            0 => {
                self.apps
                    .enter(appid, |app, _| {
                        app.read_buffer = Some(slice);
                        app.read_idx = 0;
                        ReturnCode::SUCCESS
                    })
                    .unwrap_or_else(|err| {
                        match err {
                            Error::OutOfMemory => ReturnCode::ENOMEM,
                            Error::AddressOutOfBounds => ReturnCode::EINVAL,
                            Error::NoSuchApp => ReturnCode::EINVAL,
                        }
                    })
            }
            // Allow a write buffer
            1 => {
                self.apps
                    .enter(appid, |app, _| {
                        app.write_buffer = Some(slice);
                        ReturnCode::SUCCESS
                    })
                    .unwrap_or_else(|err| {
                        match err {
                            Error::OutOfMemory => ReturnCode::ENOMEM,
                            Error::AddressOutOfBounds => ReturnCode::EINVAL,
                            Error::NoSuchApp => ReturnCode::EINVAL,
                        }
                    })
            }
            _ => ReturnCode::ENOSUPPORT,
        }
    }

    fn subscribe(&self, subscribe_num: usize, callback: Callback) -> ReturnCode {
        match subscribe_num {
            0 /* read line */ => {
                panic!("readline is borked right now. Don't call it");
                ReturnCode::FAIL
            },
            1 /* putstr/write_done */ => {
                self.apps.enter(callback.app_id(), |app, _| {
                    match app.write_buffer.take() {
                        Some(slice) => {
                            app.write_callback = Some(callback);
                            app.write_len = slice.len();
                            if self.in_progress.is_none() {
                                self.in_progress.replace(callback.app_id());
                                self.tx_buffer.take().map(|buffer| {
                                    for (i, c) in slice.as_ref().iter().enumerate() {
                                        if buffer.len() <= i {
                                            break;
                                        }
                                        buffer[i] = *c;
                                    }
                                    self.uart.transmit(buffer, app.write_len);
                                });
                            } else {
                                app.pending_write = true;
                                app.write_buffer = Some(slice);
                            }
                            ReturnCode::SUCCESS
                        },
                        None => ReturnCode::FAIL
                    }
                }).unwrap_or_else(|err| {
                    match err {
                        Error::OutOfMemory => ReturnCode::ENOMEM,
                        Error::AddressOutOfBounds => ReturnCode::EINVAL,
                        Error::NoSuchApp => ReturnCode::EINVAL,
                    }
                })
            },
            2 /* read automatic */ => {
                self.apps.enter(callback.app_id(), |app, _| {
                    app.read_callback = Some(callback);

                    // only both receiving if we've got somewhere to put it
                    app.read_buffer = app.read_buffer.take().map(|app_buf| {
                        self.rx_buffer.take().map(|buffer| {
                            let mut receive_len = app_buf.len();
                            if receive_len > buffer.len() {
                                receive_len = buffer.len();
                            }
                            //XXX: this could receive more than the app buffer...
                            self.uart.receive_automatic(buffer, 10);
                        });

                        app_buf
                    });

                    ReturnCode::SUCCESS
                }).unwrap_or_else(|err| {
                    match err {
                        Error::OutOfMemory => ReturnCode::ENOMEM,
                        Error::AddressOutOfBounds => ReturnCode::EINVAL,
                        Error::NoSuchApp => ReturnCode::EINVAL,
                    }
                })
            },
            _ => ReturnCode::ENOSUPPORT,
        }
    }

    fn command(&self, cmd_num: usize, arg1: usize, _: AppId) -> ReturnCode {
        match cmd_num {
            0 /* putc */ => {
                self.tx_buffer.take().map(|buffer| {
                    buffer[0] = arg1 as u8;
                    self.uart.transmit(buffer, 1);
                });
                ReturnCode::SuccessWithValue{ value: 1 }
            },
            _ => ReturnCode::ENOSUPPORT,
        }
    }
}

impl<'a, U: UARTAdvanced> Client for Console<'a, U> {
    fn transmit_complete(&self, buffer: &'static mut [u8], _error: uart::Error) {
        // Write TX is done, notify appropriate app and start another
        // transaction if pending
        self.tx_buffer.replace(buffer);
        self.in_progress.take().map(|appid| {
            self.apps.enter(appid, |app, _| {
                app.write_callback.map(|mut cb| {
                    cb.schedule(app.write_len, 0, 0);
                });
                app.write_len = 0;
            })
        });

        for cntr in self.apps.iter() {
            let started_tx = cntr.enter(|app, _| {
                if app.pending_write {
                    app.pending_write = false;
                    app.write_buffer
                        .as_ref()
                        .map(|slice| {
                            self.tx_buffer.take().map(|buffer| {
                                for (i, c) in slice.as_ref().iter().enumerate() {
                                    if buffer.len() <= i {
                                        break;
                                    }
                                    buffer[i] = *c;
                                }
                                self.uart.transmit(buffer, app.write_len);
                            });
                            self.in_progress.replace(app.appid());
                            true
                        })
                        .unwrap_or(false)
                } else {
                    false
                }
            });
            if started_tx {
                break;
            }
        }
    }

    fn receive_complete(&self, rx_buffer: &'static mut [u8], rx_len: usize, error: uart::Error) {
        // if error != uart::Error::CommandComplete {
        //    panic!("UART error: {:x}", error as u32);
        // }

        // always always always replace the rx buffer
        self.rx_buffer.replace(rx_buffer);

        // send line to apps if any are subscribed
        self.apps.each(|app| {
            app.read_buffer = app.read_buffer.take().map(|mut rb| {
                // copy until newline or app buffer is full
                let mut max_idx = rx_len;
                if rb.len() < rx_len {
                    max_idx = rb.len();
                }

                // copy over data to app buffer
                self.rx_buffer.map(|buffer| {
                    for idx in 0..max_idx {
                        rb.as_mut()[idx] = buffer[idx];
                    }
                });

                // call application handler
                app.read_callback.as_mut().map(|cb| {
                    let buf = rb.as_mut();
                    cb.schedule(max_idx, (buf.as_ptr() as usize), 0);
                });

                rb
            });
        });



        // old readline code
        // unsafe {sam4l::gpio::PA[18].clear();}
        //
        // if error != uart::Error::CommandComplete {
        // panic!("UART error: {:x}", error as u32);
        // }
        //
        // let c = rx_buffer[0];
        // self.rx_buffer.replace(rx_buffer);
        //
        // if line was complete, time to start overwriting it
        // if self.line_complete.get() {
        // self.line_complete.set(false);
        // self.line_idx.set(0);
        // self.line_buffer.map(|line| {
        // });
        // }
        //
        // write character to line buffer
        // self.line_buffer.map(|line| {
        // line[self.line_idx.get()] = c;
        // self.line_idx.set(self.line_idx.get()+1);
        // });
        //
        // check if the line is complete
        // if c as char == '\n' ||
        //      self.line_idx.get() >= self.line_buffer.map_or(0, |buf| buf.len()) {
        // self.line_complete.set(true);
        //
        // send line to apps if any are subscribed
        // let mut buffer_sent = Cell::new(false);
        // self.apps.each(|app| {
        // buffer_sent.set(true);
        //
        // app.read_buffer = app.read_buffer.take().map(|mut rb| {
        // self.line_buffer.map(|line| {
        // copy until newline or app buffer is full
        // let mut max_idx = self.line_idx.get();
        // if rb.len() < self.line_idx.get() {
        // max_idx = rb.len();
        // }
        //
        // copy over data to app buffer
        // for idx in 0..max_idx {
        // rb.as_mut()[idx] = line[idx];
        // }
        //
        // call application handler
        // app.read_callback.as_mut().map(|cb| {
        // let buf = rb.as_mut();
        // unsafe {sam4l::gpio::PA[25].toggle();}
        // cb.schedule(max_idx, (buf.as_ptr() as usize), 0);
        // });
        // });
        //
        // rb
        // });
        // });
        //
        // if sent to an app, clear the line so it can't get sent twice
        // if buffer_sent.get() {
        // self.line_complete.set(false);
        // self.line_idx.set(0);
        // }
        // }
        //
        // keep collecting more bytes
        // self.rx_buffer.take().map(|buffer| {
        // unsafe {sam4l::gpio::PA[18].set();}
        // self.uart.receive(buffer, 1);
        // });
        //
    }
}
