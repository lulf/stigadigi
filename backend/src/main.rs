extern crate log;

use actix_web::{web, App, HttpServer};

use stigadigi_backend::GameData;

mod api {
    //  use actix_web::dev::HttpResponseBuilder;
    //  use actix_web::http::StatusCode;
    use actix_web::{get, post, put, web, HttpRequest, HttpResponse};
    // use cloudevents_sdk_actix_web::HttpRequestExt;
    use serde::{Deserialize, Serialize};
    use stigadigi_backend::{Game, GameData, GameId, Player, PlayerId, Score};

    /// Notify API that new game should be created and assigned an id.
    /// POST /games
    ///   RESPONSE: {"id": 12}
    #[derive(Serialize, Deserialize)]
    struct NewGameResponse {
        id: GameId,
    }

    #[post("/api/v1/games")]
    async fn new_game(
        _: HttpRequest,
        data: web::Data<GameData>,
    ) -> Result<HttpResponse, actix_web::Error> {
        let result = data.new_game().await;
        match result {
            Ok(id) => Ok(HttpResponse::Ok().json(NewGameResponse { id })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }

    /// Register a new player
    /// POST /players
    ///   BODY: {"name": "Hockeyhurricane"}
    ///   RESPONSE: {"status": 201, player: {"id": 332, "name": "Hockeyhurricane"}}

    #[derive(Serialize, Deserialize)]
    struct NewPlayerRequest {
        name: String,
    }

    #[derive(Serialize, Deserialize)]
    struct NewPlayerResponse {
        id: PlayerId,
    }

    #[post("/api/v1/players")]
    async fn new_player(
        player: web::Json<NewPlayerRequest>,
        data: web::Data<GameData>,
    ) -> Result<HttpResponse, actix_web::Error> {
        let result = data.new_player(&player.name).await;
        match result {
            Ok(id) => Ok(HttpResponse::Ok().json(NewPlayerResponse { id })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }

    /// Read all players
    /// GET /players
    ///   RESPONSE: [{"id": 332, "name": "Hockeyhurricane", "rating": 1333}]

    #[derive(Serialize, Deserialize)]
    struct ListPlayersResponse {
        players: Vec<Player>,
    }

    #[get("/api/v1/players")]
    async fn list_players(data: web::Data<GameData>) -> Result<HttpResponse, actix_web::Error> {
        let result = data.list_players().await;
        match result {
            Ok(ps) => Ok(HttpResponse::Ok().json(ListPlayersResponse { players: ps })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }

    /// GET /games
    ///   RESPONSE: [{"12", "left": 332, "right": 333, state: {"left": 0, "right": 5, "status": "finished"}}]
    ///
    ///
    #[derive(Serialize, Deserialize)]
    struct ListGamesResponse {
        games: Vec<Game>,
    }

    #[get("/api/v1/games")]
    async fn list_games(data: web::Data<GameData>) -> Result<HttpResponse, actix_web::Error> {
        let result = data.list_games().await;
        match result {
            Ok(gs) => Ok(HttpResponse::Ok().json(ListGamesResponse { games: gs })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }

    /// Update game state during game
    /// PUT /games/12
    ///   BODY: {"id": 12, "state": {"left": 0, "right": 4}}
    ///   RESPONSE: {"status": 201}
    ///
    /// Update game state marking as finished
    /// PUT /games/12
    ///   BODY: {"id": 12, "state": {"left": 0, "right": 5, "status": "finished"}}
    ///   RESPONSE: {"status": 201}
    #[derive(Serialize, Deserialize)]
    struct UpdateGameResponse {
        id: GameId,
    }

    #[derive(Serialize, Deserialize)]
    struct UpdateGameRequest {
        score_left: Score,
        score_right: Score,
        status: Option<String>,
    }

    #[put("/api/v1/games/{game_id}")]
    async fn update_game(
        web::Path(game_id): web::Path<GameId>,
        game: web::Json<UpdateGameRequest>,
        data: web::Data<GameData>,
    ) -> Result<HttpResponse, actix_web::Error> {
        let result = data
            .update_game_status(
                game_id,
                game.score_left,
                game.score_right,
                game.status.clone(),
            )
            .await;
        match result {
            Ok(_) => Ok(HttpResponse::Ok().json(UpdateGameResponse { id: game_id })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }

    /// Register a game, binding the result to the player stats.
    /// PUT /games/12/register
    ///   BODY: {"id": 12, left: 332, right: 333}
    #[derive(Serialize, Deserialize)]
    struct RegisterGameRequest {
        player_left: PlayerId,
        player_right: PlayerId,
    }

    #[put("/api/v1/games/{game_id}/register")]
    async fn register_game(
        web::Path(game_id): web::Path<GameId>,
        game: web::Json<RegisterGameRequest>,
        data: web::Data<GameData>,
    ) -> Result<HttpResponse, actix_web::Error> {
        let result = data
            .update_game_players(game_id, game.player_left, game.player_right)
            .await;
        match result {
            Ok(_) => Ok(HttpResponse::Ok().json(UpdateGameResponse { id: game_id })),
            Err(e) => Ok(HttpResponse::InternalServerError().json(e.to_string())),
        }
    }
}

#[actix_rt::main]
async fn main() -> std::io::Result<()> {
    // Setup logger
    env_logger::init();

    let data = GameData::new().expect("error initializing game data");

    // Get port from envs
    let port: u16 = std::env::var("PORT")
        .ok()
        .map(|e| e.parse().ok())
        .flatten()
        .unwrap_or(8888);

    // Create the HTTP server
    HttpServer::new(move || {
        App::new()
            .wrap(actix_web::middleware::Logger::default())
            .app_data(web::Data::new(data.clone()))
            .service(api::new_game)
            .service(api::new_player)
            .service(api::list_games)
            .service(api::list_players)
            .service(api::update_game)
    })
    .bind(("127.0.0.1", port))?
    .workers(1)
    .run()
    .await
}
