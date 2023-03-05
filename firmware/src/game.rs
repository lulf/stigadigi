pub struct Game {
    log: Log,
    score: [u32; 2],
}

#[derive(defmt::Format, Copy, Clone)]
pub enum Side {
    Left,
    Right,
}

impl Side {
    pub fn goal(&self) -> Self {
        match self {
            Self::Left => Self::Right,
            Self::Right => Self::Left,
        }
    }
}

const LEFT_IDX: usize = 0;
const RIGHT_IDX: usize = 1;
const MAX_SCORE: u32 = 10;

impl Game {
    pub fn new() -> Game {
        Game {
            log: Log::new(),
            score: [0; 2],
        }
    }

    pub fn goal(&mut self, side: Side) {
        let idx = match side {
            Side::Left => LEFT_IDX,
            Side::Right => RIGHT_IDX,
        };

        if self.score[idx] < MAX_SCORE {
            self.score[idx] += 1;
        }

        let event = match side {
            Side::Left => Event::LeftGoal,
            Side::Right => Event::RightGoal,
        };
        self.log.add(event);
    }

    pub fn score(&self, side: Side) -> u32 {
        match side {
            Side::Left => self.score[LEFT_IDX],
            Side::Right => self.score[RIGHT_IDX],
        }
    }

    pub fn undo(&mut self) {
        let event = self.log.undo();
        match event {
            Event::LeftGoal => {
                // Event log should be consistent with score
                assert!(self.score[LEFT_IDX] > 0);
                self.score[LEFT_IDX] -= 1;
            }
            Event::RightGoal => {
                // Event log should be consistent with score
                assert!(self.score[RIGHT_IDX] > 0);
                self.score[RIGHT_IDX] -= 1;
            }
            Event::None => {}
        }
    }

    pub fn reset(&mut self) {
        self.log.reset();
        self.score[LEFT_IDX] = 0;
        self.score[RIGHT_IDX] = 0;
    }

    pub fn print(&self) {
        defmt::info!(
            "Game status: {} - {}",
            self.score[LEFT_IDX],
            self.score[RIGHT_IDX]
        );
    }
}

const MAX_EVENTS: usize = 20;

struct Log {
    events: [Event; MAX_EVENTS],
    position: usize,
}

#[derive(Clone, Copy)]
enum Event {
    LeftGoal,
    RightGoal,
    None,
}

impl Log {
    fn new() -> Log {
        Log {
            events: [Event::None; MAX_EVENTS],
            position: 0,
        }
    }

    fn undo(&mut self) -> Event {
        if self.position > 0 {
            self.position -= 1;
            let prev = self.events[self.position];
            self.events[self.position] = Event::None;
            prev
        } else {
            Event::None
        }
    }

    fn add(&mut self, event: Event) {
        if self.position < MAX_EVENTS {
            self.events[self.position] = event;
            self.position += 1;
        }
    }

    fn reset(&mut self) {
        self.events = [Event::None; MAX_EVENTS];
    }
}
