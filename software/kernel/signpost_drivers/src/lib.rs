#![crate_name = "signpost_drivers"]
#![crate_type = "rlib"]
#![feature(const_fn)]
#![no_std]

extern crate kernel;
extern crate signpost_hil;


pub mod mcp23008;
pub mod pca9544a;
pub mod ltc2941;
pub mod fm25cl;

pub mod lps331ap;
pub mod lps25hb;
pub mod si7021;
pub mod tsl2561;

pub mod gpio_async;
pub mod i2c_selector;
pub mod smbus_interrupt;
pub mod i2c_master_slave_driver;
pub mod uartprint;
pub mod app_watchdog;
pub mod watchdog_kernel;
