use hil;

pub trait GPIOAsyncPort {
	fn disable(&self, pin: usize) -> isize;
	fn enable_output(&self, pin: usize) -> isize;
	fn enable_input(&self, pin: usize, mode: hil::gpio::InputMode) -> isize;
	fn read(&self, pin: usize) -> isize;
	fn toggle(&self, pin: usize) -> isize;
	fn set(&self, pin: usize) -> isize;
	fn clear(&self, pin: usize) -> isize;
	fn enable_interrupt(&self, pin: usize, client_data: usize,
                        mode: hil::gpio::InterruptMode) -> isize;
    fn disable_interrupt(&self, pin: usize) -> isize;
}

pub trait Client {
    fn fired(&self, identifier: usize);
    fn done(&self, value: usize);
}
