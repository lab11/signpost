#![crate_name = "signpost_hil"]
#![crate_type = "rlib"]
#![feature(asm,lang_items,const_fn)]
#![no_std]

extern crate common;
extern crate main;
extern crate hil;

// pub mod led;
// pub mod alarm;
pub mod gpio_async;
// pub mod i2c;
// pub mod spi_master;
// pub mod timer;
// pub mod uart;
// pub mod adc;

// pub trait Controller {
//     type Config;

//     fn configure(&self, Self::Config);
// }
