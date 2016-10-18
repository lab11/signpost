/// Software based watchdog for applications.

use core::cell::Cell;

use kernel::{AppId, Driver};
use kernel::hil::alarm;
use kernel::hil::alarm::Frequency;



pub enum TimeoutMode {
    App,
    Kernel,
}

pub struct Timeout<'a, A: alarm::Alarm + 'a> {
    alarm: &'a A,
    timeout: Cell<usize>, // milliseconds before resetting the app
    mode: TimeoutMode,
    reset: unsafe fn(),
}

impl<'a, A: alarm::Alarm + 'a> Timeout<'a, A> {
    pub fn new(alarm: &'a A, mode: TimeoutMode, timeout: usize, reset: unsafe fn()) -> Timeout<'a, A> {
        Timeout {
            alarm: alarm,
            timeout: Cell::new(timeout),
            mode: mode,
            reset: reset,
        }
    }

    fn tickle(&self) {
        let interval = (self.timeout.get() as u32) * <A::Frequency>::frequency() / 1000;
        let tics = self.alarm.now().wrapping_add(interval);
        self.alarm.set_alarm(tics);
    }

    fn stop(&self) {
        self.alarm.disable_alarm();
    }
}


pub struct AppWatchdog<'a, A: alarm::Alarm + 'a> {
    app_timeout: &'a Timeout<'a, A>,
    kernel_timeout: &'a Timeout<'a, A>,
}

impl<'a, A: alarm::Alarm + 'a> AppWatchdog<'a, A> {
    pub fn new(appt: &'a Timeout<'a, A>, kernelt: &'a Timeout<'a, A>) -> AppWatchdog<'a, A> {
        AppWatchdog {
            app_timeout: appt,
            kernel_timeout: kernelt,
        }
    }

    fn set_app_timeout(&self, timeout: usize) {
        self.app_timeout.timeout.set(timeout);
    }

    fn set_kernel_timeout(&self, timeout: usize) {
        self.kernel_timeout.timeout.set(timeout);
    }

    fn start(&self) {
        self.app_timeout.tickle();
        self.kernel_timeout.tickle();
    }

    fn stop(&self) {
        self.app_timeout.stop();
        self.kernel_timeout.stop();
    }

    fn tickle_app(&self) {
        self.app_timeout.tickle();
    }

    fn tickle_kernel(&self) {
        self.kernel_timeout.tickle();
    }
}



impl<'a, A: alarm::Alarm + 'a> alarm::AlarmClient for Timeout<'a, A> {
    fn fired(&self) {
        match self.mode {
            TimeoutMode::App => {
                // reset app
            }
            TimeoutMode::Kernel => {
                // reset whole processor
                // reset();
                unsafe {
                    (self.reset)();
                }
            }
        }
    }
}

impl<'a, A: alarm::Alarm + 'a> Driver for AppWatchdog<'a, A> {
    fn command(&self, command_num: usize, data: usize, _: AppId) -> isize {
        match command_num {
            // Tickle application timer
            0 => {
                self.tickle_app();
                0
            },
            // Tickle kernel timer
            1 => {
                self.tickle_kernel();
                0
            },
            // Set app timeout in milliseconds
            2 => {
                self.set_app_timeout(data);
                0
            },
            // Set kernel timeout in milliseconds
            3 => {
                self.set_kernel_timeout(data);
                0
            },
            // Enable app watchdog
            4 => {
                self.start();
                0
            },
            // Disable app watchdog
            5 => {
                self.stop();
                0
            },

            // default
            _ => -1
        }

    }
}
