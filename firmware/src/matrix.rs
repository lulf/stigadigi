use embedded_hal::digital::v2::OutputPin;
use hal::gpio::{Level, Output, Pin, PushPull};
use log;

use nrf52833_hal as hal;

pub struct LedMatrix {
    rows: [Pin<Output<PushPull>>; 5],
    cols: [Pin<Output<PushPull>>; 5],
}

impl LedMatrix {
    pub fn new(rows: [Pin<Output<PushPull>>; 5], cols: [Pin<Output<PushPull>>; 5]) -> LedMatrix {
        let mut m = LedMatrix {
            rows: rows,
            cols: cols,
        };
        m.clear();
        m
    }

    pub fn clear(&mut self) {
        for row in self.rows.iter_mut() {
            row.set_low().unwrap();
        }
        for col in self.cols.iter_mut() {
            col.set_high().unwrap();
        }
    }

    pub fn on(&mut self, x: usize, y: usize) {
        //let (r, c) = self.coordinates[x][y];
        log::info!("Enabling {}.{}", x, y);
        self.rows[x].set_high().unwrap();
        self.cols[y].set_low().unwrap();
    }

    pub fn off(&mut self, x: usize, y: usize) {
        // let (r, c) = self.coordinates[x][y];
        self.rows[x].set_low().unwrap();
        self.cols[y].set_high().unwrap();
    }
}
