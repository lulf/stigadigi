#[macro_use]
extern crate log;

use actix_web::dev::HttpResponseBuilder;
use actix_web::http::StatusCode;
use actix_web::{post, web, App, HttpRequest, HttpResponse, HttpServer};
use cloudevents_sdk_actix_web::HttpRequestExt;
use std::collections::HashMap;
use diesel::prelude::*;
use dotenv::dotenv;
use std::env;

use stigadigi_backend::models::*;
use stigadigi_backend::establish_connection;



/// Notify API that new game should be created and assigned an id.
/// POST /games
///   RESPONSE: {"id": 12}

/// Update game state during game
/// PUT /games/12
///   BODY: {"id": 12, "state": {"left": 0, "right": 4}}
///   RESPONSE: {"status": 201}

/// Update game state marking as finished
/// PUT /games/12
///   BODY: {"id": 12, "state": {"left": 0, "right": 5, "status": "finished"}}
///   RESPONSE: {"status": 201}

/// Register a new player
/// POST /players
///   BODY: {"name": "Hockeyhurricane"}
///   RESPONSE: {"status": 201, player: {"id": 332, "name": "Hockeyhurricane"}}

/// Read specific player
/// GET /players/332
///   BODY: {"id": "332"}
///   RESPONSE: {"id": 332, "name": "Hockeyhurricane", "rating": 1333}

/// Read all players
/// GET /players
///   RESPONSE: [{"id": 332, "name": "Hockeyhurricane", "rating": 1333}]

/// Register a completed game, binding the result to the player stats.
/// PUT /games/12/register
///   BODY: {"id": 12, left: 332, right: 333}

/// GET /games
///   RESPONSE: [{"12", "left": 332, "right": 333, state: {"left": 0, "right": 5, "status": "finished"}}]
///
///
///   
///   "


struct GameController {
    connection: PgConnection,
}

impl GameController {
    fn new() -> GameController {
        let connection = establish_connection();
        GameController {
            connection
        }
    }

    fn new_game(&mut self) -> GameId {
        let id: GameId = 0;
        id
    }

    fn get_game(&mut self, id: GameId) -> Option<&mut Game> {
        None
    }
}

#[post("/")]
async fn reply_event(
    req: HttpRequest,
    payload: web::Payload,
) -> Result<HttpResponse, actix_web::Error> {
    let request_event = req.to_event(payload).await?;
    info!("{:?}", request_event);

    HttpResponseBuilder::new(StatusCode::OK).await
}


#[actix_rt::main]
async fn main() -> std::io::Result<()> {
    // Setup logger
    std::env::set_var("RUST_LOG", "info");
    env_logger::init();

    // Get port from envs
    let port: u16 = std::env::var("PORT")
        .ok()
        .map(|e| e.parse().ok())
        .flatten()
        .unwrap_or(8080);

    // Create the HTTP server
    HttpServer::new(|| {
        let app_base = App::new().wrap(actix_web::middleware::Logger::default());
        app_base.service(reply_event)
    })
    .bind(("127.0.0.1", port))?
    .workers(1)
    .run()
    .await
}
