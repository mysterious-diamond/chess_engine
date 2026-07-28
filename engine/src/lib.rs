use crate::ffi::*;
mod ffi;

#[unsafe(no_mangle)]
pub extern "C" fn get_engine_move(mut board: Board, search_depth: u8, is_team_white: bool) -> u16 {
    println!("Got function call");
    let mut alpha = f64::NEG_INFINITY;
    let beta = f64::INFINITY;

    let legal_moves: Vec<Vec<u16>> = get_all_legal_moves_of_team(&mut board, is_team_white);
    let mut chosen_move: u16 = 0;

    for piece_moves in legal_moves {
        for move_data in piece_moves {
            let mut start_board = board;

            unsafe {
                make_move(&mut start_board as *mut Board, move_data);
            }

            let score = search(start_board, search_depth, -beta, -alpha, !is_team_white);

            if score > alpha {
                alpha = score;
                chosen_move = move_data;
            }

            println!("Searched legal move branch {}", move_data);
        }
    }

    chosen_move
}

fn search(mut board: Board, search_depth: u8, mut alpha: f64, beta: f64, is_team_white: bool) -> f64 {
    if search_depth == 0 {
        unsafe {
            return evaluate_board(&mut board, is_team_white);
        }
    }

    let legal_moves: Vec<Vec<u16>> = get_all_legal_moves_of_team(&mut board, is_team_white);
    for piece_moves in legal_moves {
        for move_data in piece_moves {
            let mut new_board = board;
            unsafe {
                make_move(&mut new_board, move_data);
                if (move_data & 1) == 1 {
                    try_promote_pawn(new_board, &mut new_board.white_king)
                }
            }
            let score: f64 = -search(new_board, search_depth - 1, -beta, -alpha, !is_team_white);

            if score >= beta {
                return beta;
            }

            if score > alpha {
                alpha = score;
            }
        }
    }

    alpha
}

fn get_all_legal_moves_of_team(board: &mut Board, is_team_white: bool) -> Vec<Vec<u16>> {
    let mut legal_moves: Vec<Vec<u16>> = Vec::new();

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
            get_piece_legal_moves(board, piece_moves.as_mut_ptr(), cell);
            let piece_type = get_piece_type_on_cell(board, cell);

            get_strictly_legal_moves(board, piece_moves.as_mut_ptr(), piece_type);
        }

        legal_moves.push(piece_moves);
    }
    legal_moves
}
