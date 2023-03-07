#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![feature(async_fn_in_trait)]
#![allow(incomplete_features)]

use embedded_graphics::{
    image::{Image, ImageRaw},
    pixelcolor::BinaryColor,
    prelude::*,
};
use ssd1306::{prelude::*, I2CDisplayInterface, Ssd1306};
use {
    embassy_executor::Spawner,
    embassy_futures::select::{select, select4, Either, Either4},
    embassy_rp::{
        gpio::{AnyPin, Input, Pin, Pull},
        i2c,
    },
    embassy_time::{Duration, Instant, Ticker},
    futures::StreamExt,
};

#[cfg(feature = "panic-probe")]
use panic_probe as _;

use defmt_rtt as _;

#[cfg(feature = "panic-reset")]
use panic_reset as _;

const VERSION: &str = env!("CARGO_PKG_VERSION");

mod game;
use crate::game::*;

const COOLDOWN: Duration = Duration::from_secs(5);

pub struct Goal {
    side: Side,
    input: Input<'static, AnyPin>,
    last_goal: Instant,
}

impl Goal {
    fn new(side: Side, input: Input<'static, AnyPin>) -> Self {
        Self {
            side,
            input,
            last_goal: Instant::MIN,
        }
    }

    fn reset(&mut self) {
        self.last_goal = Instant::MIN;
    }

    async fn wait(&mut self) {
        self.input.wait_for_rising_edge().await;
        defmt::info!("[{}] event: {}", self.side, self.input.is_high());
    }

    fn check(&mut self, game: &mut Game, now: Instant) {
        if self.last_goal + COOLDOWN < now {
            defmt::info!("[{}] GOAL!!", self.side.goal());
            game.goal(self.side.goal());
            game.print();
            self.last_goal = now;
        }
    }
}

#[embassy_executor::main]
async fn main(_s: Spawner) {
    let p = embassy_rp::init(Default::default());

    defmt::info!("Version: {}", VERSION);

    let sda = p.PIN_14;
    let scl = p.PIN_15;

    let mut i2c = i2c::I2c::new_blocking(p.I2C1, scl, sda, i2c::Config::default());

    let interface = I2CDisplayInterface::new(i2c);
    let mut display = Ssd1306::new(interface, DisplaySize128x64, DisplayRotation::Rotate0)
        .into_buffered_graphics_mode();
    display.init().unwrap();

    let raw: ImageRaw<BinaryColor> = ImageRaw::new(include_bytes!("./rust.raw"), 64);

    let im = Image::new(&raw, Point::new(32, 0));

    im.draw(&mut display).unwrap();

    display.flush().unwrap();

    let input_left = Input::new(p.PIN_4.degrade(), Pull::Up);
    let input_right = Input::new(p.PIN_5.degrade(), Pull::Up);

    let mut left = Goal::new(Side::Left, input_left);
    let mut right = Goal::new(Side::Right, input_right);

    let mut start_btn = Input::new(p.PIN_26.degrade(), Pull::Up);
    let mut undo_btn = Input::new(p.PIN_27.degrade(), Pull::Up);

    let mut game = Game::new();

    loop {
        match select4(
            left.wait(),
            right.wait(),
            start_btn.wait_for_falling_edge(),
            undo_btn.wait_for_falling_edge(),
        )
        .await
        {
            Either4::First(_) => {
                left.check(&mut game, Instant::now());
            }
            Either4::Second(_) => {
                right.check(&mut game, Instant::now());
            }
            Either4::Third(res) => {
                defmt::info!("Game reset!");
                game.reset();
                left.reset();
                right.reset();
            }
            Either4::Fourth(_) => {
                defmt::info!("Game undo!");
                game.undo();
                game.print();
            }
        }
    }
}
