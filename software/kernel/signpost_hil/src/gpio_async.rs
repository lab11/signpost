use kernel;
use kernel::returncode::ReturnCode;

pub trait GPIOAsyncPort {
	fn disable(&self, pin: usize) -> ReturnCode;
	fn enable_output(&self, pin: usize) -> ReturnCode;
	fn enable_input(&self, pin: usize, mode: kernel::hil::gpio::InputMode) -> ReturnCode;
	fn read(&self, pin: usize) -> ReturnCode;
	fn toggle(&self, pin: usize) -> ReturnCode;
	fn set(&self, pin: usize) -> ReturnCode;
	fn clear(&self, pin: usize) -> ReturnCode;
	fn enable_interrupt(&self, pin: usize, client_data: usize,
                        mode: kernel::hil::gpio::InterruptMode) -> ReturnCode;
    fn disable_interrupt(&self, pin: usize) -> ReturnCode;
}

pub trait Client {
    fn fired(&self, identifier: usize);
    fn done(&self, value: usize);
}
