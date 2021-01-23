#![no_std]
#![no_main]

use nrf52833_hal as hal;
use panic_halt as _;

extern crate cortex_m;

use embedded_hal::digital::v2::InputPin;
use embedded_hal::digital::v2::OutputPin;

use core::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use hal::gpio::{Input, Level, Output, Pin, PullUp, PushPull};
use hal::gpiote::*;
use hal::rtc::{Rtc, RtcCompareReg, RtcInterrupt};
use rtic::app;

use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

mod game;
use crate::game::*;

mod matrix;
use crate::matrix::*;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Trace);
const COOLDOWN: u32 = 500;

pub struct Goal {
    input: Pin<Input<PullUp>>,
    active: bool,
    last_goal: u32,
}

impl Goal {
    fn reset(&mut self) {
        self.active = false;
        self.last_goal = 0;
    }

    fn is_active(&mut self) -> bool {
        self.input.is_high().unwrap()
    }
}

#[app(device = crate::hal::pac, peripherals = true)]
const APP: () = {
    struct Resources {
        gpiote: Gpiote,
        left: Goal,
        right: Goal,
        game: Game,
        led: LedMatrix,
        rtc: Rtc<hal::pac::RTC0>,
        #[init(AtomicU32::new(0))]
        ticks: AtomicU32,
        #[init(AtomicBool::new(false))]
        started: AtomicBool,
    }

    #[init]
    fn init(ctx: init::Context) -> init::LateResources {
        rtt_init_print!();

        unsafe {
            log::set_logger_racy(&LOGGER).unwrap();
        }
        log::set_max_level(log::LevelFilter::Trace);

        let p0 = hal::gpio::p0::Parts::new(ctx.device.P0);
        let p1 = hal::gpio::p1::Parts::new(ctx.device.P1);

        let start_btn = p0.p0_14.into_pullup_input().degrade();
        let undo_btn = p0.p0_23.into_pullup_input().degrade();

        let input_left = p0.p0_03.into_pullup_input().degrade();
        let input_right = p0.p0_02.into_pullup_input().degrade();

        let gpiote = Gpiote::new(ctx.device.GPIOTE);
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
        gpiote
            .channel2()
            .input_pin(&start_btn)
            .hi_to_lo()
            .enable_interrupt();
        gpiote
            .channel3()
            .input_pin(&undo_btn)
            .hi_to_lo()
            .enable_interrupt();

        let led = LedMatrix::new(
            [
                p0.p0_21.into_push_pull_output(Level::Low).degrade(),
                p0.p0_22.into_push_pull_output(Level::Low).degrade(),
                p0.p0_15.into_push_pull_output(Level::Low).degrade(),
                p0.p0_24.into_push_pull_output(Level::Low).degrade(),
                p0.p0_19.into_push_pull_output(Level::Low).degrade(),
            ],
            [
                p0.p0_28.into_push_pull_output(Level::Low).degrade(),
                p0.p0_11.into_push_pull_output(Level::Low).degrade(),
                p0.p0_31.into_push_pull_output(Level::Low).degrade(),
                p1.p1_05.into_push_pull_output(Level::Low).degrade(),
                p0.p0_30.into_push_pull_output(Level::Low).degrade(),
            ],
        );

        let left = Goal {
            input: input_left,
            active: false,
            last_goal: 0,
        };

        let right = Goal {
            input: input_right,
            active: false,
            last_goal: 0,
        };

        let mut rtc = Rtc::new(ctx.device.RTC0, 68).unwrap();
        rtc.enable_event(RtcInterrupt::Compare0);
        let _ = rtc.set_compare(RtcCompareReg::Compare0, 2);

        let game = Game::new();

        log::info!("Initialized application");
        init::LateResources {
            gpiote,
            left,
            right,
            rtc,
            game,
            led,
        }
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        log::info!("Started application");
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = RTC0, resources = [rtc, ticks, started, game, left, right, led])]
    fn on_rtc(ctx: on_rtc::Context) {
        let on_rtc::Resources {
            rtc,
            ticks,
            started,
            game,
            left,
            right,
            led,
        } = ctx.resources;

        let now = ticks.fetch_add(1, Ordering::SeqCst);

        if started.load(Ordering::SeqCst) {
            led.clear();
            for i in 0..game.score(Side::Left) {
                led.on(i as usize % 5, (i as usize / 5) % 5);
            }

            for i in 0..game.score(Side::Right) {
                led.on(i as usize % 5, 4 - ((i as usize / 5) % 5));
            }
            check_game(game, left, right, now);
        }
        led.process();
        rtc.reset_event(RtcInterrupt::Compare0);
        rtc.clear_counter();
    }

    #[task(binds = GPIOTE, resources = [gpiote, left, right, rtc, started, game, ticks, led])]
    fn on_detected(ctx: on_detected::Context) {
        let on_detected::Resources {
            gpiote,
            left,
            right,
            started,
            game,
            ticks,
            rtc,
            led,
        } = ctx.resources;

        if gpiote.channel0().is_event_triggered() || gpiote.channel1().is_event_triggered() {
            check_game(game, left, right, ticks.load(Ordering::SeqCst));
        } else if gpiote.channel2().is_event_triggered() {
            if !started.load(Ordering::SeqCst) {
                log::info!("Starting game!");
                // TODO: Call game API
                started.store(true, Ordering::SeqCst);
                rtc.enable_interrupt(RtcInterrupt::Compare0, None);
                rtc.reset_event(RtcInterrupt::Compare0);
                rtc.clear_counter();
                rtc.enable_counter();
            } else {
                log::info!("Stopping game!");
                // TODO: Call game API
                started.store(false, Ordering::SeqCst);
                ticks.store(0, Ordering::SeqCst);
                game.reset();
                right.reset();
                led.clear();
                left.reset();
                rtc.disable_counter();
                rtc.disable_interrupt(RtcInterrupt::Compare0, None);
            }
        } else if gpiote.channel3().is_event_triggered() {
            if started.load(Ordering::SeqCst) {
                log::info!("Calling undo!");
                game.undo();
                game.print();
            }
        }
        gpiote.reset_events();
    }

    extern "C" {
        fn WDT();
    }
};

fn check_game(game: &mut Game, left: &mut Goal, right: &mut Goal, now: u32) {
    if now - left.last_goal > COOLDOWN {
        if left.is_active() && !left.active {
            log::info!("GOAL PLAYER LEFT!!");
            game.goal(Side::Right);
            game.print();
            left.active = true;
        } else if !left.is_active() && left.active {
            log::info!("Left: Starting cooldown period");
            left.last_goal = now;
            left.active = false;
        }
    }

    if now - right.last_goal > COOLDOWN {
        if right.is_active() && !right.active {
            log::info!("GOAL PLAYER RIGHT!!");
            game.goal(Side::Left);
            game.print();
            right.active = true;
        } else if !right.is_active() && right.active {
            log::info!("Right: Starting cooldown period");
            right.last_goal = now;
            right.active = false;
        }
    }
}
