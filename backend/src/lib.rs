#[macro_use]
extern crate diesel;
extern crate dotenv;
extern crate r2d2;

use diesel::prelude::*;
use diesel::r2d2::{ConnectionManager, Pool};
use std::convert::From;
use std::env;
use std::error;
use std::fmt;

pub mod models;
pub mod schema;

pub use self::models::{Game, GameId, NewGame, NewPlayer, Player, PlayerId, Score};

fn establish_connection() -> Result<Pool<ConnectionManager<PgConnection>>, r2d2::Error> {
    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    Pool::builder().build(manager)
}

#[derive(Clone)]
pub struct GameData {
    connection: Pool<ConnectionManager<PgConnection>>,
}

impl GameData {
    pub fn new() -> Result<GameData, BackendError> {
        let connection = establish_connection()?;
        Ok(GameData { connection })
    }

    pub async fn new_game(&self) -> Result<GameId, BackendError> {
        use schema::games;
        let conn = self.connection.get()?;

        let new_game = NewGame {
            status: "Started",
            score_left: 0,
            score_right: 0,
        };

        let game: Game = diesel::insert_into(games::table)
            .values(&new_game)
            .get_result(&conn)?;
        Ok(game.id)
    }

    pub async fn update_game_status(
        &self,
        game_id: GameId,
        updated_score_left: Score,
        updated_score_right: Score,
        updated_status: Option<String>,
    ) -> Result<Game, BackendError> {
        use schema::games::dsl::*;
        let conn = self.connection.get()?;

        let game: Game = match updated_status {
            Some(s) => diesel::update(games.filter(id.eq(game_id)))
                .set((
                    score_left.eq(updated_score_left),
                    score_right.eq(updated_score_right),
                    status.eq(s),
                ))
                .get_result(&conn)?,
            None => diesel::update(games.filter(id.eq(game_id)))
                .set((
                    score_left.eq(updated_score_left),
                    score_right.eq(updated_score_right),
                ))
                .get_result(&conn)?,
        };
        Ok(game)
    }

    pub async fn update_game_players(
        &self,
        game_id: GameId,
        p_left: PlayerId,
        p_right: PlayerId,
    ) -> Result<Game, BackendError> {
        use schema::games::dsl::*;
        let conn = self.connection.get()?;

        let game: Game = diesel::update(games.filter(id.eq(game_id)))
            .set((player_left.eq(p_left), player_right.eq(p_right)))
            .get_result(&conn)?;
        Ok(game)
    }

    pub async fn new_player(&self, name: &str) -> Result<PlayerId, BackendError> {
        use schema::players;
        let conn = self.connection.get()?;

        let new_player = NewPlayer {
            name: name.to_string(),
            rating: 1500,
        };

        let player: Player = diesel::insert_into(players::table)
            .values(&new_player)
            .get_result(&conn)?;
        Ok(player.id)
    }

    pub async fn list_games(&self) -> Result<Vec<Game>, BackendError> {
        use schema::games::dsl::*;
        let conn = self.connection.get()?;

        Ok(games.load::<Game>(&conn)?)
    }

    pub async fn list_players(&self) -> Result<Vec<Player>, BackendError> {
        use schema::players::dsl::*;
        let conn = self.connection.get()?;

        Ok(players.load::<Player>(&conn)?)
    }

    /*
    pub async fn get_game(&mut self, id: GameId) -> Result<&mut Game, BackendError> {
        Ok(Game {
            id: 0,
            score_left: 0,
            score_right: 0,
            status: "Completed".to_string()
    }
    */
}

#[derive(Debug, Clone)]
pub struct BackendError {
    message: String,
}

impl error::Error for BackendError {}

impl fmt::Display for BackendError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.message)
    }
}

impl From<r2d2::Error> for BackendError {
    fn from(e: r2d2::Error) -> Self {
        BackendError {
            message: e.to_string(),
        }
    }
}

impl From<diesel::result::Error> for BackendError {
    fn from(e: diesel::result::Error) -> Self {
        BackendError {
            message: e.to_string(),
        }
    }
}

/*
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_insert() {
        let c = establish_connection();
        let p = create_player(&c);
        println!("Result: {:?}", p);
    }
}
*/
