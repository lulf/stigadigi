#![no_std]
#![no_main]

use nrf52833_hal as hal;
use panic_halt as _;

extern crate cortex_m;
use cortex_m_rt::entry;
use log::LevelFilter;
use rtt_logger::RTTLogger;
use rtt_target::rtt_init_print;

static LOGGER: RTTLogger = RTTLogger::new(LevelFilter::Info);

#[entry]
fn main() -> ! {
    rtt_init_print!();

    log::set_logger(&LOGGER).unwrap();
    log::set_max_level(log::LevelFilter::Info);

    log::info!("Started application");

    loop {}
}
