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
        return quiescence(board, alpha, beta, last_move, is_team_white);
    }

    let legal_moves: Vec<u16> = get_all_legal_moves_of_team(&mut board, last_move, is_team_white);
    let mut legal_moves: Vec<u16> = legal_moves.into_iter().filter(|&m| m != 0).collect();
    legal_moves.sort_by_key(|m| -score_move(m, &mut board));
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

fn quiescence(mut board: Board, mut alpha: f64, beta: f64, last_move: u16, is_team_white: bool) -> f64 {
    let stand_pat = evaluate_board(&mut board, is_team_white);

    if stand_pat >= beta {
        return beta;
    }

    if stand_pat > alpha {
        alpha = stand_pat;
    }

    let moves: Vec<u16> = get_all_legal_moves_of_team(&mut board, last_move, is_team_white);
    let captures: Vec<u16> = moves.into_iter().filter(|&m| m & 1 == 1).collect();

    for mv in captures {
        let mut new_board = board;

        unsafe {
            make_move(&mut new_board, last_move, mv);
        }

        let score = -quiescence(new_board, -beta, -alpha, mv, !is_team_white);

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
    for cell in 0..64 {
        let piece_type: *mut u64;
        unsafe {
            piece_type = get_piece_type_on_cell(board, cell);
        }

        if piece_type.is_null() {
            continue;
        }

        score += get_piece_value(board, cell) as f64;
    }

    score += 20.0;
    score -= get_double_pawn_penalty(board, is_evaluating_white);
    if is_evaluating_white { score } else { -score }
}

fn score_move(move_data: &u16, board: &mut Board) -> i64 {
    let is_capture = (move_data & 1) != 0;
    if !is_capture {
        return 0;
    }

    let from = ((move_data >> 10) & 63) as u8;
    let to = ((move_data >> 4) & 63) as u8;

    let victim = get_piece_value(board, to);
    let attacker = get_piece_value(board, from);

    victim * 10 - attacker
}

fn get_piece_value(board: &mut Board, cell: u8) -> i64 {
    get_pst_score_of_piece(board, cell) + get_material_score_of_piece(board, cell)
}

fn get_pst_score_of_piece(board: &mut Board, cell: u8) -> i64 {
    let pst_pawn = vec![
        0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30, 20, 10, 10, 5, 5, 10, 25, 25, 10,
        5, 5, 0, 0, 0, 20, 20, 0, 0, 0, 5, -5, -10, 0, 0, -10, -5, 5, 5, 10, 10, -20, -20, 10, 10, 5, 0, 0, 0, 0, 0, 0,
        0, 0,
    ];
    let pst_rook = vec![
        0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5,
        0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0,
    ];
    let pst_knight = vec![
        -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 0, 0, 0, -20, -40, -30, 0, 10, 15, 15, 10, 0, -30, -30, 5,
        15, 20, 20, 15, 5, -30, -30, 0, 15, 20, 20, 15, 0, -30, -30, 5, 10, 15, 15, 10, 5, -30, -40, -20, 0, 5, 5, 0,
        -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
    ];
    let pst_bishop = vec![
        -20, -10, -10, -10, -10, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 5, 5,
        10, 10, 5, 5, -10, -10, 0, 10, 10, 10, 10, 0, -10, -10, 10, 10, 10, 10, 10, 10, -10, -10, 5, 0, 0, 0, 0, 5,
        -10, -20, -10, -10, -10, -10, -10, -10, -20,
    ];
    let pst_queen = vec![
        -20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -5, 0, 5, 5, 5,
        5, 0, -5, 0, 0, 5, 5, 5, 5, 0, -5, -10, 5, 5, 5, 5, 5, 0, -10, -10, 0, 5, 0, 0, 0, 0, -10, -20, -10, -10, -5,
        -5, -10, -10, -20,
    ];
    let pst_early_game_king: Vec<i16> = vec![
        -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30,
        -30, -20, -10, -20, -20, -20, -20, -20, -20, -10, 20, 20, 0, 0, 0, 0, 20, 20, 20, 30, 10, 0, 0, 10, 30, 20,
    ];
    let pst_late_game_king: Vec<i16> = vec![
        -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0, 0, -10, -20, -30, -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -30, 0, 0, 0, 0, -30, -30, -50, -30, -30, -30, -30, -30, -30, -50,
    ];

    let is_white: bool;
    unsafe {
        is_white = is_piece_on_cell_white(board as *mut Board, cell);
    }
    let pst_index = if is_white { cell } else { 63 - cell };
    let game_phase = get_game_phase(board) as i16;
    let piece_type: u64;
    unsafe {
        piece_type = *get_piece_type_on_cell(board, cell);
    }

    if piece_type == board.white_pawns || piece_type == board.black_pawns {
        pst_pawn[pst_index as usize]
    } else if piece_type == board.white_rooks || piece_type == board.black_rooks {
        pst_rook[pst_index as usize]
    } else if piece_type == board.white_knights || piece_type == board.black_knights {
        pst_knight[pst_index as usize]
    } else if piece_type == board.white_bishops || piece_type == board.black_bishops {
        pst_bishop[pst_index as usize]
    } else if piece_type == board.white_queens || piece_type == board.black_queens {
        pst_queen[pst_index as usize]
    } else if piece_type == board.white_king || piece_type == board.black_king {
        ((pst_early_game_king[pst_index as usize] * (256 - game_phase) / 256)
            + (pst_late_game_king[pst_index as usize] * game_phase / 256)) as i64
    } else {
        0
    }
}

fn get_material_score_of_piece(board: &mut Board, cell: u8) -> i64 {
    let pawn_material_score = 100;
    let rook_material_score = 500;
    let knight_material_score = 320;
    let bishop_material_score = 330;
    let queen_material_score = 900;

    let piece_type: u64;
    unsafe {
        piece_type = *get_piece_type_on_cell(board as *mut Board, cell);
    }

    if piece_type == board.white_pawns || piece_type == board.black_pawns {
        pawn_material_score
    } else if piece_type == board.white_rooks || piece_type == board.black_rooks {
        rook_material_score
    } else if piece_type == board.white_knights || piece_type == board.black_knights {
        knight_material_score
    } else if piece_type == board.white_bishops || piece_type == board.black_bishops {
        bishop_material_score
    } else if piece_type == board.white_queens || piece_type == board.black_queens {
        queen_material_score
    } else {
        0
    }
}

fn get_game_phase(board: &Board) -> u16 {
    let mut pawns = 0;
    let mut rooks = 0;
    let mut knights = 0;
    let mut bishops = 0;
    let mut queens = 0;

    for i in 0..64 {
        if board.white_pawns & (1u64 << i) != 0 || board.black_pawns & (1u64 << i) != 0 {
            pawns += 1;
        }
        if board.white_rooks & (1u64 << i) != 0 || board.black_rooks & (1u64 << i) != 0 {
            rooks += 1;
        }
        if board.white_knights & (1u64 << i) != 0 || board.black_knights & (1u64 << i) != 0 {
            knights += 1;
        }
        if board.white_bishops & (1u64 << i) != 0 || board.black_bishops & (1u64 << i) != 0 {
            bishops += 1;
        }
        if board.white_queens & (1u64 << i) != 0 || board.black_queens & (1u64 << i) != 0 {
            queens += 1;
        }
    }

    let mut game_phase = pawns + knights + bishops + rooks * 2 + queens * 4;
    game_phase = 24 - game_phase;
    game_phase = game_phase.clamp(0, 24);

    // Scale from 0-24 range to 0-256 range proportionally
    game_phase * (256 / 24)
}

fn get_double_pawn_penalty(board: &Board, is_checking_white: bool) -> f64 {
    let mut penalty: f64 = 0.0;
    let pawns: u64 = if is_checking_white {
        board.white_pawns
    } else {
        board.black_pawns
    };

    for file in 0..8 {
        let count = pawns & (0x0101010101010101 << file);
        let count = count.count_ones();
        if count > 1 {
            penalty += (count - 1) as f64 * 20.0;
        }
    }

    penalty
}

fn get_all_legal_moves_of_team(board: &mut Board, last_move: u16, is_team_white: bool) -> Vec<u16> {
    let mut legal_moves: Vec<u16> = Vec::new();

    for cell in 0..64 {
        unsafe {
            let is_empty: bool = is_cell_empty(board as *mut Board, cell);
            let is_same_team: bool = is_piece_on_cell_white(board, cell) == is_team_white;
            if is_empty || !is_same_team {
                continue;
            }
        }

        let mut piece_moves: Vec<u16> = vec![0u16; 27];
        unsafe {
            get_piece_legal_moves(board as *mut Board, piece_moves.as_mut_ptr(), last_move, cell, 0);
            let piece_type = get_piece_type_on_cell(board, cell);

            get_strictly_legal_moves(board as *mut Board, last_move, piece_moves.as_mut_ptr(), piece_type);
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
