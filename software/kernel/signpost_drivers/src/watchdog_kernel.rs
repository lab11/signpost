/// Timer loop to keep watchdog from firing

use core::cell::Cell;

use kernel::hil;
use kernel::hil::alarm;
use kernel::hil::alarm::Frequency;



pub struct WatchdogKernel<'a, A: alarm::Alarm + 'a> {
    alarm: &'a A,
    watchdog: &'a hil::watchdog::Watchdog,
    timeout: Cell<usize>, // milliseconds before resetting the app
}

impl<'a, A: alarm::Alarm + 'a> WatchdogKernel<'a, A> {
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

        self.watchdog.start();
    }
}

impl<'a, A: alarm::Alarm + 'a> alarm::AlarmClient for WatchdogKernel<'a, A> {
    fn fired(&self) {
        self.watchdog.tickle();

        let interval = (self.timeout.get() as u32) * <A::Frequency>::frequency() / 1000;
        let tics = self.alarm.now().wrapping_add(interval);
        self.alarm.set_alarm(tics);
    }
}
