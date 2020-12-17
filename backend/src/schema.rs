table! {
    games (id) {
        id -> Nullable<Integer>,
        score_left -> Integer,
        score_right -> Integer,
        status -> Text,
        player_left -> Nullable<Integer>,
        player_right -> Nullable<Integer>,
    }
}

table! {
    players (id) {
        id -> Nullable<Integer>,
        rating -> Integer,
    }
}

allow_tables_to_appear_in_same_query!(
    games,
    players,
);
