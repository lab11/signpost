/// Timer loop to keep watchdog from firing

use core::cell::Cell;

use kernel::hil;
use kernel::hil::time;
use kernel::hil::time::Frequency;



pub struct WatchdogKernel<'a, A: time::Alarm + 'a> {
    alarm: &'a A,
    watchdog: &'a hil::watchdog::Watchdog,
    timeout: Cell<usize>, // milliseconds before resetting the app
}

impl<'a, A: time::Alarm + 'a> WatchdogKernel<'a, A> {
    pub fn new(alarm: &'a A, watchdog: &'a hil::watchdog::Watchdog, timeout: usize) -> WatchdogKernel<'a, A> {
        WatchdogKernel {
            alarm: alarm,
            watchdog: watchdog,
            timeout: Cell::new(timeout),
        }
    }

    pub fn start(&self) {
        let interval = (self.timeout.get() as u32) * <A::Frequency>::frequency() / 1000;
        let tics = self.alarm.now().wrapping_add(interval);
        self.alarm.set_alarm(tics);

        self.watchdog.start(2000);
    }
}

impl<'a, A: time::Alarm + 'a> time::Client for WatchdogKernel<'a, A> {
    fn fired(&self) {
        self.watchdog.tickle();

        let interval = (self.timeout.get() as u32) * <A::Frequency>::frequency() / 1000;
        let tics = self.alarm.now().wrapping_add(interval);
        self.alarm.set_alarm(tics);
    }
}
