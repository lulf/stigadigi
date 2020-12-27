#![no_std]
#![no_main]

use nrf52833_hal as hal;
use panic_halt as _;

extern crate cortex_m;

use embedded_hal::digital::v2::InputPin;
use embedded_hal::digital::v2::OutputPin;

use hal::gpio::{Input, Level, Output, Pin, PullUp, PushPull};
use hal::gpiote::*;
use hal::rtc::{Rtc, RtcCompareReg, RtcInterrupt};
use rtic::app;

use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

mod game;
use crate::game::*;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Trace);
const COOLDOWN: u32 = 8;

pub struct Goal {
    input: Pin<Input<PullUp>>,
    //led: Pin<Output<PushPull>>,
    active: u32,
}

impl Goal {
    fn reset(&mut self) {
        self.active = 0;
        //  self.led.set_high().unwrap();
    }

    fn is_active(&mut self) -> bool {
        self.input.is_low().unwrap()
    }
}

#[app(device = crate::hal::pac, peripherals = true)]
const APP: () = {
    struct Resources {
        gpiote: Gpiote,
        left: Goal,
        right: Goal,
        game: Game,
        rtc: Rtc<hal::pac::RTC0>,
        #[init(0)]
        now: u32,
        #[init(false)]
        started: bool,
    }

    #[init]
    fn init(ctx: init::Context) -> init::LateResources {
        rtt_init_print!();

        unsafe {
            log::set_logger_racy(&LOGGER).unwrap();
        }
        log::set_max_level(log::LevelFilter::Trace);

        let gpio = hal::gpio::p0::Parts::new(ctx.device.P0);

        let start_btn = gpio.p0_14.into_pullup_input().degrade();
        let undo_btn = gpio.p0_23.into_pullup_input().degrade();

        let input_left = gpio.p0_03.into_pullup_input().degrade();
        let input_right = gpio.p0_02.into_pullup_input().degrade();

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

        let left = Goal {
            input: input_left,
            //led: gpio.p0_21.into_push_pull_output(Level::High).degrade(),
            active: 0,
        };

        let right = Goal {
            input: input_right,
            //led: gpio.p0_24.into_push_pull_output(Level::High).degrade(),
            active: 0,
        };

        let mut rtc = Rtc::new(ctx.device.RTC0, 4095).unwrap();
        rtc.enable_event(RtcInterrupt::Compare0);
        let _ = rtc.set_compare(RtcCompareReg::Compare0, 10);

        let game = Game::new();

        log::info!("Initialized application");
        init::LateResources {
            gpiote,
            left,
            right,
            rtc,
            game,
        }
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        log::info!("Started application");
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = RTC0, resources = [rtc, left, right, now, started])]
    fn on_rtc(ctx: on_rtc::Context) {
        let on_rtc::Resources {
            rtc,
            left,
            right,
            now,
            started,
        } = ctx.resources;

        *now += 1;
        let nownow = *now;

        if *started {
            if left.active > 0 && nownow - left.active > COOLDOWN && !left.is_active() {
                log::trace!("RESETTING LEFT COOLDOWN");
                //left.led.set_high().unwrap();
                left.active = 0;
            }
            if right.active > 0 && nownow - right.active > COOLDOWN && !right.is_active() {
                log::trace!("RESETTING RIGHT COOLDOWN");
                //  right.led.set_high().unwrap();
                right.active = 0;
            }
        }
        rtc.reset_event(RtcInterrupt::Compare0);
        rtc.clear_counter();
    }

    #[task(binds = GPIOTE, resources = [gpiote, left, right, rtc, started, game, now])]
    fn on_detected(ctx: on_detected::Context) {
        let on_detected::Resources {
            gpiote,
            left,
            right,
            started,
            game,
            now,
            rtc,
        } = ctx.resources;

        if gpiote.channel0().is_event_triggered() {
            log::trace!("INTERRUPT LEFT: {}", left.is_active());
            if *started && left.active == 0 && left.is_active() {
                log::info!("GOAL PLAYER LEFT!!");
                //  left.led.set_low().unwrap();
                game.goal(Side::Left);
                game.print();
                left.active = *now;
            }
        } else if gpiote.channel1().is_event_triggered() {
            log::trace!("INTERRUPT RIGHT: {}", right.is_active());
            if *started && right.active == 0 && right.is_active() {
                log::info!("GOAL PLAYER RIGHT!!");
                //right.led.set_low().unwrap();
                game.goal(Side::Right);
                game.print();
                right.active = *now;
            }
        } else if gpiote.channel2().is_event_triggered() {
            if !*started {
                log::info!("Starting game!");
                // TODO: Call game API
                *started = true;
                rtc.enable_interrupt(RtcInterrupt::Compare0, None);
                rtc.reset_event(RtcInterrupt::Compare0);
                rtc.clear_counter();
                rtc.enable_counter();
            } else {
                log::info!("Stopping game!");
                // TODO: Call game API
                *started = false;
                game.reset();
                right.reset();
                left.reset();
                rtc.disable_counter();
                rtc.disable_interrupt(RtcInterrupt::Compare0, None);
            }
        } else if gpiote.channel3().is_event_triggered() {
            if *started {
                log::info!("Calling undo!");
                game.undo();
                game.print();
            }
        }
        gpiote.reset_events();
    }
};
