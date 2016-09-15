// This should get added to hil::spi_master at some point, hence the clunky
// name.


use hil;

pub trait SPIMasterDevice {
    // fn init(&mut self, client: &'static SpiCallback);
    // fn is_busy(&self) -> bool;


    fn configure(&self, cpol: hil::spi_master::ClockPolarity, cpal: hil::spi_master::ClockPhase, rate: u32);

    /// Perform an asynchronous read/write operation, whose
    /// completion is signaled by invoking SpiCallback on
    /// the initialzied client. write_buffer must be Some,
    /// read_buffer may be None. If read_buffer is Some, the
    /// length of the operation is the minimum of the size of
    /// the two buffers.
    fn read_write_bytes(&self,
                        mut write_buffer: Option<&'static mut [u8]>,
                        mut read_buffer: Option<&'static mut [u8]>,
                        len: usize)
                        -> bool;

}
