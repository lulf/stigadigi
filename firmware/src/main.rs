#![no_std]
#![no_main]

use nrf51_hal as hal;
use panic_halt as _;

extern crate cortex_m;

use hal::gpio::{Input, Pin, PullUp};
use hal::gpiote::*;
use rtic::app;

use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Info);

#[app(device = crate::hal::pac, peripherals = true)]
const APP: () = {
    struct Resources {
        gpiote: Gpiote,
        input: Pin<Input<PullUp>>,
    }

    #[init]
    fn init(mut ctx: init::Context) -> init::LateResources {
        rtt_init_print!();

        unsafe {
            log::set_logger_racy(&LOGGER).unwrap();
        }
        log::set_max_level(log::LevelFilter::Info);

        let gpio = hal::gpio::p0::Parts::new(ctx.device.GPIO);

        let mut input = gpio.p0_01.into_pullup_input().degrade();

        //    let _clocks = Clocks::new(p.CLOCK).enable_ext_hfosc();

        let mut gpiote = Gpiote::new(ctx.device.GPIOTE);
        gpiote
            .channel0()
            .input_pin(&input)
            .hi_to_lo()
            .enable_interrupt();

        log::info!("Initialized application");
        init::LateResources { gpiote, input }
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        log::info!("Started application");
        loop {
            cortex_m::asm::wfi();
        }
    }

    #[task(binds = GPIOTE, resources = [gpiote])]
    fn on_detected(ctx: on_detected::Context) {
        if ctx.resources.gpiote.channel0().is_event_triggered() {
            log::info!("GOAL!!");
        }
        ctx.resources.gpiote.reset_events();
    }
};
