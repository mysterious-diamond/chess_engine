use crate::ffi::*;
mod ffi;

#[unsafe(no_mangle)]
pub extern "C" fn get_engine_move(mut board: Board, search_depth: u8, last_move: u16, is_team_white: bool) -> u16 {
    let mut alpha = f64::NEG_INFINITY;
    let beta = f64::INFINITY;

    let legal_moves: Vec<u16> = get_all_legal_moves_of_team(&mut board, last_move, is_team_white);

    let mut legal_moves: Vec<u16> = legal_moves.into_iter().filter(|&m| m != 0).collect();
    legal_moves.sort_by_key(|m| if m & 1 == 1 { 0 } else { 1 });
    let mut chosen_move: u16 = 0;

    for move_data in legal_moves {
        let mut start_board = board;
        unsafe {
            make_move(&mut start_board as *mut Board, last_move, move_data);
        }

        let score = search(start_board, search_depth - 1, -beta, -alpha, last_move, !is_team_white);

        if score > alpha {
            alpha = score;
            chosen_move = move_data;
        }
    }

    chosen_move
}

fn search(mut board: Board, search_depth: u8, mut alpha: f64, beta: f64, last_move: u16, is_team_white: bool) -> f64 {
    if search_depth == 0 {
        unsafe {
            return evaluate_board(&mut board, is_team_white);
        }
    }

    let legal_moves: Vec<u16> = get_all_legal_moves_of_team(&mut board, last_move, is_team_white);
    let mut legal_moves: Vec<u16> = legal_moves.into_iter().filter(|&m| m != 0).collect();
    legal_moves.sort_by_key(|m| if m & 1 == 1 { 0 } else { 1 });
    if legal_moves.is_empty() {
        return f64::NEG_INFINITY;
    }

    for move_data in legal_moves {
        if move_data == 0 {
            continue;
        }

        let mut new_board = board;
        unsafe {
            make_move(&mut new_board, last_move, move_data);
            if (move_data & 4) == 4 {
                let _ = try_promote_pawn(&mut new_board as *mut Board, 1);
            }
        }

        let score: f64 = -search(new_board, search_depth - 1, -beta, -alpha, move_data, !is_team_white);

        if score >= beta {
            return beta;
        }

        if score > alpha {
            alpha = score;
        }
    }

    alpha
}

fn evaluate_board(board: &mut Board, is_evaluating_white: bool) -> f64 {
    let mut score: f64 = 0.0;
    for i in 0..64 {
        let piece_type: *mut u64;
        unsafe {
            piece_type = get_piece_type_on_cell(board, i);
        }

        if piece_type.is_null() {
            continue;
        }

        let material_score = get_material_score_of_piece(board, i);
        let PST_score = get_PST_score_of_piece(board, i);

        score += material_score + PST_score;
    }

    if is_evaluating_white {score} else {-score}
}

fn get_PST_score_of_piece(board: &Board, cell: u8) -> f64 {
    let pawnPST = vec![0,  0,   0,  0, 0,  0,  0,  0,   50,  50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30,  20,
                             10, 10,  5,  5, 10, 25, 25, 10,  5,   5,  0,  0,  0,  20, 20, 0,  0,  0,  5,  -5, -10, 0,
                             0,  -10, -5, 5, 5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,  0,  0,  0,  0,  0];
    let rookPST = vec![0, 0,  0,  0,  0,  0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0,  0,  0, 0, 0,
                             0, -5, -5, 0,  0,  0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0, 0,  -5, -5, 0, 0, 0,
                             0, 0,  0,  -5, -5, 0, 0, 0, 0, 0,  0,  -5, 0,  0,  0,  5, 5,  0,  0,  0];
    let knightPST = vec![-50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,   0,   -20, -40,
                               -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,   15,  20,  20,  15,  5,   -30,
                               -30, 0,   15,  20,  20,  15,  0,   -30, -30, 5,   10,  15,  15,  10,  5,   -30,
                               -40, -20, 0,   5,   5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50];
    let bishopPST = vec![-20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,   0,   0,   -10,
                               -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,   5,   10,  10,  5,   5,   -10,
                               -10, 0,   10,  10,  10,  10,  0,   -10, -10, 10,  10,  10,  10,  10,  10,  -10,
                               -10, 5,   0,   0,   0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20];
    let queenPST = vec![-20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,  0, 0,   0,   0,   0,   -10, -10, 0,   5,   5,  5, 5,
                              0,   -10, -5,  0,   5,   5,   5,   5,   0,   -5, 0, 0,   5,   5,   5,   5,   0,   -5,  -10, 5,  5, 5,
                              5,   5,   0,   -10, -10, 0,   5,   0,   0,   0,  0, -10, -20, -10, -10, -5,  -5,  -10, -10, -20];
    let kingEarlyMidGamePST = vec![-30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
                                         -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20, -20, -20, -20, -10,
                                         20,  20,  0,   0,   0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20];
    let kingEndGamePST = vec![-50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,   -10, -20, -30,
                                    -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30,
                                    -30, -10, 30,  40,  40,  30,  -10, -30, -30, -10, 20,  30,  30,  20,  -10, -30,
                                    -30, -30, 0,   0,   0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50];

    let is_white: bool; 
    unsafe {is_white = is_piece_on_cell_white(board as *mut Board, cell);}
    let pst_index = if is_white {cell} else {63 - cell};
    let game_phase = get_game_phase(board);
    let piece_type = *get_piece_type_on_cell(board, cell);
    let score;

    if piece_type == board.white_pawns || piece_type == board.black_pawns {score = pawnPST[pst_index];}
    if (piece_type == board.white_rooks || piece_type == board.black_rooks) {score = rookPST[pst_index];}
    if (piece_type == board.white_knights || piece_type == board.black_knights) {score = knightPST[pst_index];}
    if (piece_type == board.white_bishops || piece_type == board.black_bishops) {score = bishopPST[pst_index];}
    if (piece_type == board.white_queens || piece_type == board.black_queens) {score = queenPST[pst_index];}
    if (piece_type == board.white_king || piece_type == board.black_king) {
        score = kingEarlyMidGamePST[pst_index] * (256 - game_phase) / 256;
        score += kingEndGamePST[pst_index] * game_phase / 256;
    }

    score
}

fn get_material_score_of_piece(board: &Board, cell: u8) -> f64 {
    let pawnMaterialScore = 100;
    let rookMaterialScore = 500;
    let knightMaterialScore = 320;
    let bishopMaterialScore = 330;
    let queenMaterialScore = 900;

    let piece_type = *get_piece_type_on_cell(board, cell);
    let score = 0.0;

    if (piece_type == board.white_pawns || piece_type == board.black_pawns) {score += pawnMaterialScore;}
    if (piece_type == board.white_rooks || piece_type == board.black_rooks) {score += rookMaterialScore;}
    if (piece_type == board.white_knights || piece_type == board.black_knights) {score += knightMaterialScore;}
    if (piece_type == board.white_bishops || piece_type == board.black_bishops) {score += bishopMaterialScore;}
    if (piece_type == board.white_queens || piece_type == board.black_queens) {score += queenMaterialScore;}

    score
}

fn get_game_phase(board: Board) -> u16 {
    let pawns = 0;
    let rooks = 0;
    let knights = 0;
    let bishops = 0;
    let queens = 0;

    for i in 0..64 {
        if board.white_pawns & (1u64 << i) != 0 || board.black_pawns & (1u64 << i) != 0 {pawns += 1;}
        if board.white_rooks & (1u64 << i) != 0 || board.black_rooks & (1u64 << i) != 0 {rooks += 1;}
        if board.white_knights & (1u64 << i) != 0 || board.black_knights & (1u64 << i) != 0 {knights += 1;}
        if board.white_bishops & (1u64 << i) != 0 || board.black_bishops & (1u64 << i) != 0 {bishops += 1;}
        if board.white_queens & (1u64 << i) != 0 || board.black_queens & (1u64 << i) != 0 {queens += 1;}
    }

    int gamePhase = pawns + knights + bishops + rooks * 2 + queens * 4;
    gamePhase = 24 - gamePhase;
    if (gamePhase > 24) gamePhase = 24;
    if (gamePhase < 0) gamePhase = 0;

    // Scale from 0-24 range to 0-256 range proportionally
    gamePhase = gamePhase * (256 / 24);
    return gamePhase;
}

fn get_all_legal_moves_of_team(board: &mut Board, last_move: u16, is_team_white: bool) -> Vec<u16> {
    let mut legal_moves: Vec<u16> = Vec::new();

    for cell in 0..64 {
        unsafe {
            let is_empty: bool = is_cell_empty(board, cell);
            let is_same_team: bool = is_piece_on_cell_white(board, cell) == is_team_white;
            if is_empty || !is_same_team {
                continue;
            }
        }

        let mut piece_moves: Vec<u16> = vec![0u16; 27];
        unsafe {
            get_piece_legal_moves(board, piece_moves.as_mut_ptr(), last_move, cell, 0);
            let piece_type = get_piece_type_on_cell(board, cell);

            get_strictly_legal_moves(board, last_move, piece_moves.as_mut_ptr(), piece_type);
        }

        for move_data in piece_moves {
            if move_data == 0 {
                continue;
            }

            legal_moves.push(move_data);
        }
    }

    legal_moves
}
