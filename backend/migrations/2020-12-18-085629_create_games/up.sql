-- Your SQL goes here
CREATE TABLE games (
    id SERIAL PRIMARY KEY,
    score_left INTEGER NOT NULL,
    score_right INTEGER NOT NULL,
    status VARCHAR NOT NULL,
    player_left INTEGER,
    player_right INTEGER
)
