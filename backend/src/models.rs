use crate::schema::games;
use crate::schema::players;
use diesel::{Insertable, Queryable};
use serde::{Deserialize, Serialize};

pub type PlayerId = i32;
pub type Rating = i32;

#[derive(Queryable, Debug, Serialize, Deserialize)]
pub struct Player {
    pub id: PlayerId,
    pub name: String,
    pub rating: Rating,
}

#[derive(Insertable, Debug)]
#[table_name = "players"]
pub struct NewPlayer {
    pub name: String,
    pub rating: Rating,
}

pub type GameId = i32;
pub type Score = i32;

#[derive(Queryable, Debug, Serialize, Deserialize)]
pub struct Game {
    pub id: GameId,
    pub score_left: Score,
    pub score_right: Score,
    pub status: String,
    pub player_left: Option<PlayerId>,
    pub player_right: Option<PlayerId>,
}

#[derive(Insertable)]
#[table_name = "games"]
pub struct NewGame<'a> {
    pub score_left: Score,
    pub score_right: Score,
    pub status: &'a str,
}
