use crate::schema::games;
use crate::schema::players;
use diesel::sql_types::Text;
use diesel::{Expression, Insertable, Queryable};

pub type PlayerId = i32;
pub type Rating = i32;

#[derive(Queryable, Debug)]
pub struct Player {
    pub id: PlayerId,
    pub rating: Rating,
}

#[derive(Insertable, Debug)]
#[table_name = "players"]
pub struct NewPlayer {
    pub rating: Rating,
}

pub type GameId = i32;

#[derive(SqlType, Debug, AsExpression)]
pub enum GameStatus {
    Started,
    Finished,
}

impl Expression for GameStatus {
    type SqlType = Text;
}

pub type Score = i32;

#[derive(Queryable)]
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
pub struct NewGame {
    pub score_left: Score,
    pub score_right: Score,
    pub status: String,
}
