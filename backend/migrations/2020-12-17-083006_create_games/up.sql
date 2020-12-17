-- Your SQL goes here
CREATE TABLE games (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    score_left int NOT NULL,
    score_right int NOT NULL,
    status VARCHAR NOT NULL,
    player_left int,
    player_right int
)
