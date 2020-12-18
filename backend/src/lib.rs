#[macro_use]
extern crate diesel;
extern crate dotenv;

use diesel::deserialize::FromSql;
use diesel::prelude::*;
use dotenv::dotenv;
use std::env;

pub mod models;
pub mod schema;

use self::models::{Game, NewGame, NewPlayer, Player};

pub fn establish_connection() -> PgConnection {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    PgConnection::establish(&database_url).expect(&format!("Error connecting to {}", database_url))
}

pub fn create_player(conn: &PgConnection) -> Player {
    use schema::players;

    let new_player = NewPlayer { rating: 1500 };

    diesel::insert_into(players::table)
        .values(&new_player)
        .get_result(conn)
        .expect("Error storing new player")
}

pub fn create_game(conn: &PgConnection) -> Game {
    use schema::games;

    let new_game = NewGame {
        status: "Started".to_string(),
        score_left: 0,
        score_right: 0,
    };

    diesel::insert_into(games::table)
        .values(&new_game)
        .get_result(conn)
        .expect("Error saving new game")
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
