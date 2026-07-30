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
