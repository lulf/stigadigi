use embedded_hal::digital::v2::OutputPin;
use hal::gpio::{Output, Pin, PushPull};

use nrf52833_hal as hal;

pub struct LedMatrix {
    buffer: [[bool; 5]; 5],
    rows: [Pin<Output<PushPull>>; 5],
    cols: [Pin<Output<PushPull>>; 5],
    row: usize,
}

impl LedMatrix {
    pub fn new(rows: [Pin<Output<PushPull>>; 5], cols: [Pin<Output<PushPull>>; 5]) -> LedMatrix {
        LedMatrix {
            buffer: [[false; 5]; 5],
            rows: rows,
            cols: cols,
            row: 0,
        }
    }

    pub fn clear(&mut self) {
        self.buffer = [[false; 5]; 5];
    }

    pub fn on(&mut self, x: usize, y: usize) {
        self.buffer[x][y] = true;
    }

    pub fn off(&mut self, x: usize, y: usize) {
        self.buffer[x][y] = false;
    }

    pub fn process(&mut self) {
        for row in self.rows.iter_mut() {
            row.set_low().unwrap();
        }

        let mut cid = 0;
        for col in self.cols.iter_mut() {
            if self.buffer[self.row][cid] {
                col.set_low().unwrap();
            } else {
                col.set_high().unwrap();
            }
            cid += 1;
        }
        self.rows[self.row].set_high().unwrap();
        self.row = (self.row + 1) % 5;
    }
}
