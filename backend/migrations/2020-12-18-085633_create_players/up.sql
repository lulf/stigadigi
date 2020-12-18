-- Your SQL goes here
CREATE TABLE players (
    id SERIAL PRIMARY KEY,
    name VARCHAR(256) NOT NULL,
    rating INTEGER NOT NULL
)
