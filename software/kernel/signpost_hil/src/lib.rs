#![crate_name = "signpost_hil"]
#![crate_type = "rlib"]
#![feature(asm,lang_items,const_fn)]
#![no_std]

extern crate common;
extern crate main;
extern crate hil;

pub mod gpio_async;
pub mod i2c_selector;

pub mod spi_master2;
