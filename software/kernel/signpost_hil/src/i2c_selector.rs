
pub trait I2CSelector {
    fn select_channels(&self, channels: usize);
    fn disable_all_channels(&self);
    fn read_interrupts(&self);
    fn read_selected(&self);
}

pub trait Client {
    //fn interrupts(&self, identifier: usize, interrupt_bitmask: usize);
    fn done(&self, Option<usize>);
}
