#![no_std]
#![no_main]

use nrf51_hal as hal;
use panic_halt as _;

extern crate cortex_m;
use cortex_m_rt::entry;

use embedded_hal::adc::OneShot;

use hal::adc::{Adc, AdcConfig};
use hal::clocks::Clocks;
use hal::rtc::{Rtc, RtcInterrupt};

use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Info);

#[entry]
fn main() -> ! {
    rtt_init_print!();

    unsafe {
        log::set_logger_racy(&LOGGER).unwrap();
    }
    log::set_max_level(log::LevelFilter::Info);

    let p = hal::pac::Peripherals::take().unwrap();
    let gpio = hal::gpio::p0::Parts::new(p.GPIO);

    let mut input = gpio.p0_01.into_floating_input();

    let config = AdcConfig {
        input_selection: hal::pac::adc::config::INPSEL_A::ANALOGINPUTNOPRESCALING,
        resolution: hal::pac::adc::config::RES_A::_10BIT,
        reference: hal::pac::adc::config::REFSEL_A::VBG,
    };
    let mut adc = Adc::new(p.ADC, config);

    let clocks = Clocks::new(p.CLOCK).enable_ext_hfosc();
    let _clocks = clocks.start_lfclk();

    let mut rtc = Rtc::new(p.RTC0, 4095).unwrap();
    rtc.enable_event(RtcInterrupt::Tick);
    rtc.enable_counter();

    log::info!("Started application");

    loop {
        while !rtc.is_event_triggered(RtcInterrupt::Tick) {}
        rtc.reset_event(RtcInterrupt::Tick);
        rtc.clear_counter();
        let value = adc.read(&mut input).unwrap();
        log::info!("Read value {}", value);
    }
}
