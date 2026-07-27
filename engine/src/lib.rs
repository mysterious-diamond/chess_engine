use crate::ffi::*;
mod engine;
mod ffi;

pub fn get_engine_move(board: Board, search_depth: u8, is_team_white: bool) -> u16 {
    let mut test_board
    search(
        &mut board,
        search_depth,
        f64::NEG_INFINITY,
        f64::INFINITY,
        is_team_white,
    );
    0
}

fn search(board: &mut Board, search_depth: u8, mut alpha: f64, mut beta: f64, is_team_white: bool) -> f64 {
    if search_depth == 0 {
        unsafe {
            return evaluate_board(board, is_team_white);
        }
    }

    let legal_moves: Vec<Vec<u16>> = get_all_legal_moves_of_team(board, is_team_white);
    for piece_moves in legal_moves {
        for move_data in piece_moves {
            unsafe {
                make_move(board, move_data);
            }
            let score: f64 = -search(board, search_depth - 1, -alpha, -beta, is_team_white);
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

        let mut piece_moves: Vec<u16> = Vec::new();
        unsafe {
            get_piece_legal_moves(board, piece_moves.as_mut_ptr(), cell);
            let mut piece_type: u64 = *get_piece_type_on_cell(board, cell);

            get_strictly_legal_moves(board, piece_moves.as_mut_ptr(), &mut piece_type);
        }

        legal_moves.push(piece_moves);
    }
    legal_moves
}
