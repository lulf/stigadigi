table! {
    games (id) {
        id -> Int4,
        score_left -> Int4,
        score_right -> Int4,
        status -> Varchar,
        player_left -> Nullable<Int4>,
        player_right -> Nullable<Int4>,
    }
}

table! {
    players (id) {
        id -> Int4,
        name -> Varchar,
        rating -> Int4,
    }
}

allow_tables_to_appear_in_same_query!(games, players,);
