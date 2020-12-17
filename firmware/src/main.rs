#![no_std]
#![no_main]

use nrf51_hal as hal;
use panic_halt as _;

extern crate cortex_m;

use embedded_hal::digital::v2::InputPin;
use embedded_hal::digital::v2::OutputPin;

use core::mem;
use hal::gpio::{Input, Level, Output, Pin, PullUp, PushPull};
use hal::gpiote::*;
use hal::pac::TIMER0;
use rtic::app;

use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Trace);
const COOLDOWN: u32 = 10_000_000u32;

pub struct Goal {
    input: Pin<Input<PullUp>>,
    led: Pin<Output<PushPull>>,
    active: u32,
}

#[app(device = crate::hal::pac, peripherals = true)]
const APP: () = {
    struct Resources {
        gpiote: Gpiote,
        left: Goal,
        right: Goal,
        timer: TIMER0,
        #[init(0)]
        now: u32,
    }

    #[init]
    fn init(mut ctx: init::Context) -> init::LateResources {
        rtt_init_print!();

        unsafe {
            log::set_logger_racy(&LOGGER).unwrap();
        }
        log::set_max_level(log::LevelFilter::Trace);

        let gpio = hal::gpio::p0::Parts::new(ctx.device.GPIO);

        let mut input_left = gpio.p0_01.into_pullup_input().degrade();
        let mut input_right = gpio.p0_07.into_pullup_input().degrade();

        //    let _clocks = Clocks::new(p.CLOCK).enable_ext_hfosc();

        let mut gpiote = Gpiote::new(ctx.device.GPIOTE);
        gpiote
            .channel0()
            .input_pin(&input_left)
            .hi_to_lo()
            .enable_interrupt();

        gpiote
            .channel1()
            .input_pin(&input_right)
            .hi_to_lo()
            .enable_interrupt();

        let left = Goal {
            input: input_left,
            led: gpio.p0_21.into_push_pull_output(Level::High).degrade(),
            active: 0,
        };

        let right = Goal {
            input: input_right,
            led: gpio.p0_24.into_push_pull_output(Level::High).degrade(),
            active: 0,
        };

        let mut timer = ctx.device.TIMER0;
        timer.init();
        timer.set_interrupt(1000000);

        log::info!("Initialized application");
        init::LateResources {
            gpiote,
            left,
            right,
            timer,
        }
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        log::info!("Started application");
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = TIMER0, resources = [timer, left, right, now])]
    fn on_timer(ctx: on_timer::Context) {
        let on_timer::Resources {
            timer,
            left,
            right,
            now,
        } = ctx.resources;

        *now = timer.now();

        let nownow = *now;

        if timer.is_pending() {
            if left.active > 0 && nownow - left.active > COOLDOWN {
                log::trace!("RESETTING LEFT COOLDOWN");
                left.led.set_high().unwrap();
                left.active = 0;
            }
            if right.active > 0 && nownow - right.active > COOLDOWN {
                log::trace!("RESETTING RIGHT COOLDOWN");
                right.led.set_high().unwrap();
                right.active = 0;
            }
        }
        timer.clear_interrupt();
        timer.set_interrupt(nownow + 1000000);
    }

    #[task(binds = GPIOTE, resources = [gpiote, left, right, timer])]
    fn on_detected(ctx: on_detected::Context) {
        let on_detected::Resources {
            gpiote,
            timer,
            left,
            right,
        } = ctx.resources;

        if gpiote.channel0().is_event_triggered() {
            log::trace!("INTERRUPT LEFT");
            if left.active == 0 {
                log::info!("GOAL PLAYER LEFT!!");
                left.led.set_low().unwrap();
                left.active = timer.now();
            }
        } else if gpiote.channel1().is_event_triggered() {
            log::trace!("INTERRUPT RIGHT");
            if right.active == 0 {
                log::info!("GOAL PLAYER RIGHT!!");
                right.led.set_low().unwrap();
                right.active = timer.now();
            }
        }
        gpiote.reset_events();
    }
};

pub trait Timer {
    unsafe fn duplicate(&self) -> Self;

    /// Initialize the timer so that it counts at a rate of 1 MHz.
    fn init(&mut self);

    /// Configures the timer's interrupt to fire at the given `Instant`.
    fn set_interrupt(&mut self, at: u32);

    /// Disables or acknowledges this timer's interrupt.
    fn clear_interrupt(&mut self);

    /// Returns whether a timer interrupt is currently pending.
    ///
    /// This must be called by the interrupt handler to avoid spurious timer events.
    fn is_pending(&self) -> bool;

    /// Obtains the current time as an `Instant`.
    fn now(&self) -> u32;
}

// Taken from Rubble BLE timer implementation.
impl Timer for TIMER0 {
    unsafe fn duplicate(&self) -> Self {
        mem::transmute_copy(self)
    }

    fn init(&mut self) {
        self.bitmode.write(|w| w.bitmode()._32bit());
        // 2^4 = 16
        // 16 MHz / 16 = 1 MHz = µs resolution
        self.prescaler.write(|w| unsafe { w.prescaler().bits(4) });
        self.tasks_clear.write(|w| unsafe { w.bits(1) });
        self.tasks_start.write(|w| unsafe { w.bits(1) });
    }

    fn set_interrupt(&mut self, at: u32) {
        self.cc[1].write(|w| unsafe { w.bits(at) });
        self.events_compare[1].reset();
        self.intenset.write(|w| w.compare1().set());
    }

    fn clear_interrupt(&mut self) {
        self.intenclr.write(|w| w.compare1().clear());
        self.events_compare[1].reset();
    }

    fn is_pending(&self) -> bool {
        self.events_compare[1].read().bits() == 1u32
    }

    fn now(&self) -> u32 {
        self.tasks_capture[0].write(|w| unsafe { w.bits(1) });
        let micros = self.cc[0].read().bits();
        micros
    }
}
