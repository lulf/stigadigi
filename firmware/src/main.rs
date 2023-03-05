#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![feature(async_fn_in_trait)]
#![allow(incomplete_features)]

use {
    embassy_executor::Spawner,
    embassy_futures::select::{select, select4, Either, Either4},
    embassy_nrf::gpio::{AnyPin, Input, Pin, Pull},
    embassy_time::{Duration, Instant, Ticker},
    futures::StreamExt,
    microbit_bsp::{display::*, *},
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
    let board = Microbit::default();
    defmt::info!("Version: {}", VERSION);

    let mut start_btn = board.btn_a;
    let mut undo_btn = board.btn_b;

    let input_left = Input::new(board.p1.degrade(), Pull::Up);
    let input_right = Input::new(board.p2.degrade(), Pull::Up);

    let mut left = Goal::new(Side::Left, input_left);
    let mut right = Goal::new(Side::Right, input_right);

    let mut display = board.display;
    display.set_brightness(Brightness::MAX);
    let mut render = Ticker::every(Duration::from_micros(100));
    let mut game = Game::new();

    loop {
        match select4(
            left.wait(),
            right.wait(),
            select(
                start_btn.wait_for_falling_edge(),
                undo_btn.wait_for_falling_edge(),
            ),
            render.next(),
        )
        .await
        {
            Either4::First(_) => {
                left.check(&mut game, Instant::now());
            }
            Either4::Second(_) => {
                right.check(&mut game, Instant::now());
            }
            Either4::Third(res) => match res {
                Either::First(_) => {
                    defmt::info!("Game reset!");
                    game.reset();
                    left.reset();
                    right.reset();
                    display.clear();
                }
                Either::Second(_) => {
                    defmt::info!("Game undo!");
                    game.undo();
                    game.print();
                }
            },
            Either4::Fourth(_) => {
                display.clear();
                for i in 0..game.score(Side::Left) {
                    display.on((i as usize / 5) % 5, i as usize % 5);
                }

                for i in 0..game.score(Side::Right) {
                    display.on(4 - ((i as usize / 5) % 5), i as usize % 5);
                }
                display.render();
            }
        }
    }
}
