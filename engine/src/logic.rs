use std::ptr::null_mut;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Board {
    pub white_pawns: u64,
    pub white_rooks: u64,
    pub white_knights: u64,
    pub white_bishops: u64,
    pub white_queens: u64,
    pub white_king: u64,
    pub black_pawns: u64,
    pub black_rooks: u64,
    pub black_knights: u64,
    pub black_bishops: u64,
    pub black_queens: u64,
    pub black_king: u64,
}

impl Default for Board {
    fn default() -> Self {
        Board {
            white_pawns: 0x000000000000FF00,
            white_rooks: 0x0000000000000081,
            white_knights: 0x0000000000000042,
            white_bishops: 0x0000000000000024,
            white_queens: 0x0000000000000008,
            white_king: 0x0000000000000010,

            black_pawns: 0x00FF000000000000,
            black_rooks: 0x8100000000000000,
            black_knights: 0x4200000000000000,
            black_bishops: 0x2400000000000000,
            black_queens: 0x0800000000000000,
            black_king: 0x1000000000000000,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn is_in_check(board: *mut Board, is_checking_white: bool) -> bool {
    let mut king_cell: u8 = 64;

    let check_king: u64 = unsafe {
        if is_checking_white {
            (*board).white_king
        } else {
            (*board).black_king
        }
    };
    for cell in 0..64 {
        if check_king & (1 << cell) != 0 {
            king_cell = cell;
            break;
        }
    }

    for cell in 0..64 {
        let legal_moves: Vec<u16> = vec![0; 27];
        get_piece_legal_moves(board, legal_moves[0] as *mut u16, 0, cell, 0);

        for move_data in legal_moves {
            if move_data == 0 {
                break;
            }
            let destination: u8 = ((move_data >> 4) & 63) as u8;

            if destination == king_cell {
                return true;
            }
        }
    }

    false
}

#[unsafe(no_mangle)]
pub extern "C" fn get_piece_legal_moves(
    board: *mut Board,
    array_ptr: *mut u16,
    last_move: u16,
    cell: u8,
    castle_flags: u8,
) {
    if cell > 63 {
        return;
    }

    let piece_type: u64 = unsafe { *get_piece_type_on_cell(board, cell) };
    let mut legal_moves: Vec<u16> = vec![0; 27];

    let mut board: Board = unsafe { *board };
    if piece_type == board.white_pawns || piece_type == board.black_pawns {
        get_pawn_legal_moves(&mut board, &mut legal_moves, last_move, cell);
    } else if piece_type == board.white_rooks || piece_type == board.black_rooks {
        get_rook_legal_moves(&mut board, &mut legal_moves, cell);
    } else if piece_type == board.white_knights || piece_type == board.black_knights {
        get_knight_legal_moves(&mut board, &mut legal_moves, cell);
    } else if piece_type == board.white_bishops || piece_type == board.black_bishops {
        get_bishop_legal_moves(&mut board, &mut legal_moves, cell);
    } else if piece_type == board.white_queens || piece_type == board.black_queens {
        get_queen_legal_moves(&mut board, &mut legal_moves, cell);
    } else if piece_type == board.white_king || piece_type == board.black_king {
        get_king_legal_moves(&mut board, &mut legal_moves, cell, castle_flags);
    }

    for (i, val) in legal_moves.into_iter().enumerate() {
        unsafe {
            *array_ptr.add(i) = val;
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn get_strictly_legal_moves(
    board: *mut Board,
    last_move: u16,
    array_ptr: *mut u16,
    piece_type: *mut u64,
) {
    let mut strictly_legal_moves: Vec<u16> = vec![0; 27];

    let mut amount_of_moves: usize = 0;
    for i in 0..27 {
        let move_data: u16 = unsafe { *array_ptr.add(i) };
        if move_data == 0 {
            break;
        }

        let origin: u8 = (move_data >> 10u64) as u8;
        let is_white: bool = is_piece_on_cell_white(board, origin);
        let is_check: bool = is_in_check(board, is_white);

        let is_castle: bool = move_data & 4 == 4;
        if is_castle && is_check {
            continue;
        }

        let does_move_cause_check: bool =
            try_make_move_and_check_if_causes_check(board, piece_type, last_move, move_data);
        if does_move_cause_check {
            continue;
        }

        strictly_legal_moves[amount_of_moves] = move_data;
        amount_of_moves += 1;
    }

    for (i, val) in strictly_legal_moves.into_iter().enumerate() {
        unsafe {
            *array_ptr.add(i) = val;
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn try_make_move_and_check_if_causes_check(
    board: *mut Board,
    piece_type: *mut u64,
    last_move: u16,
    move_data: u16,
) -> bool {
    let original_pos: u8 = (move_data >> 10 & 63) as u8;
    let new_pos: u8 = (move_data >> 4 & 63) as u8;

    let is_en_passant: bool = move_data & 8 == 8;
    let is_castle: bool = move_data & 4 == 4;
    let is_checking_white: bool = is_piece_on_cell_white(board, original_pos);

    let attack_pos: u8 = if is_en_passant {
        (last_move >> 4 & 63) as u8
    } else {
        new_pos
    };
    let attacked_piece_type: *mut u64 = get_piece_type_on_cell(board, attack_pos);

    make_move(board, last_move, move_data);
    let is_check: bool = is_in_check(board, is_checking_white);

    unsafe {
        *piece_type ^= 1 << original_pos;
    }
    unsafe {
        *piece_type ^= 1 << new_pos;
    }

    if is_castle {
        if original_pos == new_pos + 2 {
            let rook_type: *mut u64 = get_piece_type_on_cell(board, new_pos + 1);

            unsafe {
                *rook_type ^= 1 << (new_pos + 1);
            }
            unsafe {
                *rook_type ^= 1 << (new_pos - 2);
            }
        } else if original_pos == new_pos - 2 {
            let rook_type: *mut u64 = get_piece_type_on_cell(board, new_pos - 1);

            unsafe {
                *rook_type ^= 1 << (new_pos - 1);
            }
            unsafe {
                *rook_type ^= 1 << (new_pos + 1);
            }
        }
    } else if !attacked_piece_type.is_null() {
        unsafe {
            *attacked_piece_type ^= 1 << attack_pos;
        }
    }

    is_check
}

#[unsafe(no_mangle)]
pub extern "C" fn make_move(board: *mut Board, last_move: u16, move_data: u16) {
    let original_pos: u8 = (move_data >> 10) as u8 & 63;
    let new_pos: u8 = (move_data >> 4) as u8 & 63;

    let is_en_passant: bool = (move_data & 8) == 8;
    let is_castle: bool = (move_data & 4) == 4;
    let _is_promotion: bool = (move_data & 2) == 2;
    let is_capture: bool = (move_data & 1) == 1;

    let piece_type: *mut u64 = get_piece_type_on_cell(board, original_pos);

    if is_capture {
        let attack_pos: u8 = if is_en_passant { (last_move >> 4) as u8 } else { new_pos };
        remove_piece_on_cell(&mut unsafe { *board }, attack_pos);
    } else if is_castle {
        let rook_type: *mut u64;
        if original_pos == new_pos + 2 {
            rook_type = get_piece_type_on_cell(board, new_pos - 2);

            unsafe {
                *rook_type ^= 1u64 << (new_pos - 2);
            }
            unsafe {
                *rook_type ^= 1u64 << (new_pos + 1);
            }
        } else if original_pos == new_pos - 2 {
            rook_type = get_piece_type_on_cell(board, new_pos + 1);

            unsafe {
                *rook_type ^= 1u64 << (new_pos + 1);
            }
            unsafe {
                *rook_type ^= 1u64 << (new_pos - 1);
            }
        }
    }

    unsafe {
        *piece_type ^= 1u64 << original_pos;
        *piece_type ^= 1u64 << new_pos;
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn get_piece_type_on_cell(board: *mut Board, cell: u8) -> *mut u64 {
    let cell: u64 = cell as u64;

    let board: Board = unsafe { *board };

    if board.white_pawns & cell != 0 {
        board.white_pawns as *mut u64
    } else if board.white_rooks & cell != 0 {
        board.white_rooks as *mut u64
    } else if board.white_knights & cell != 0 {
        board.white_knights as *mut u64
    } else if board.white_bishops & cell != 0 {
        board.white_bishops as *mut u64
    } else if board.white_queens & cell != 0 {
        board.white_queens as *mut u64
    } else if board.white_king & cell != 0 {
        board.white_king as *mut u64
    } else {
        null_mut()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn try_promote_pawn(board: *mut Board, chosen_promotion_type: u8) -> bool {
    let pawn_promotion_cell: u8 = unsafe { get_promotion_pawn_cell(&mut *board) };
    if pawn_promotion_cell == 64 {
        return false;
    }

    let is_white: bool = is_piece_on_cell_white(board, pawn_promotion_cell);

    unsafe {
        let piece_type: &mut u64 = if is_white {
            &mut (*board).white_pawns
        } else {
            &mut (*board).black_pawns
        };
        *piece_type ^= 1 << pawn_promotion_cell;
        if chosen_promotion_type == 1 {
            let queen_type: *mut u64 = if is_white {
                (*board).white_queens as *mut u64
            } else {
                (*board).black_queens as *mut u64
            };
            *queen_type ^= 1 << pawn_promotion_cell;
        } else if chosen_promotion_type == 2 {
            let rook_type: *mut u64 = if is_white {
                (*board).white_rooks as *mut u64
            } else {
                (*board).black_rooks as *mut u64
            };
            *rook_type ^= 1 << pawn_promotion_cell;
        } else if chosen_promotion_type == 3 {
            let knight_type: *mut u64 = if is_white {
                (*board).white_knights as *mut u64
            } else {
                (*board).black_knights as *mut u64
            };
            *knight_type ^= 1 << pawn_promotion_cell;
        } else if chosen_promotion_type == 4 {
            let bishop_type: *mut u64 = if is_white {
                (*board).white_bishops as *mut u64
            } else {
                (*board).black_bishops as *mut u64
            };
            *bishop_type ^= 1 << pawn_promotion_cell;
        } else {
            return false;
        }
    }

    true
}

#[unsafe(no_mangle)]
pub extern "C" fn is_cell_empty(board: *mut Board, cell: u8) -> bool {
    get_piece_type_on_cell(board, cell).is_null()
}

#[unsafe(no_mangle)]
pub extern "C" fn is_piece_on_cell_white(board: *mut Board, cell: u8) -> bool {
    let piece_type: u64 = unsafe { *get_piece_type_on_cell(board, cell) };

    let board: Board = unsafe { *board };

    piece_type == board.white_pawns
        || piece_type == board.white_rooks
        || piece_type == board.white_knights
        || piece_type == board.white_bishops
        || piece_type == board.white_queens
        || piece_type == board.white_king
}

#[unsafe(no_mangle)]
pub extern "C" fn get_promotion_pawn_cell(board: &mut Board) -> u8 {
    for i in 56..64 {
        if board.white_pawns & (1 << i) != 0 {
            return i;
        }
    }

    for i in 0..7 {
        if board.black_pawns & (1 << i) != 0 {
            return i;
        }
    }

    64
}

fn get_pawn_legal_moves(board: &mut Board, result_array: &mut [u16], last_move: u16, cell: u8) {
    let mut legal_moves: Vec<u16> = vec![0; 27];
    let is_white: bool = is_piece_on_cell_white(board, cell);

    let direction: i8 = if is_white { 1 } else { -1 };
    let rank: u8 = cell / 8;
    let mut move_n: usize = 0;

    let mut target_cell: i8 = cell as i8 + 8 * direction;
    if is_cell_empty(board, cell) && is_valid_cell_and_not_same_team(board, target_cell as u8, is_white) {
        let is_cell_on_last_rank: bool = if is_white {
            target_cell / 8 == 7
        } else {
            target_cell / 8 == 0
        };

        legal_moves[move_n] = generate_move(cell, target_cell as u8, false, false, is_cell_on_last_rank, false);
        move_n += 1;

        target_cell = cell as i8 + 16 * direction;
        let is_pawn_at_start: bool = if is_white { rank == 1 } else { rank == 6 };

        // if pawn has never moved before, it can move 2 steps forward
        if is_pawn_at_start
            && is_cell_empty(board, target_cell as u8)
            && is_valid_cell_and_not_same_team(board, target_cell as u8, is_white)
        {
            legal_moves[move_n] = generate_move(cell, target_cell as u8, false, false, false, false);
            move_n += 1;
        }
    }

    // Left top square checking
    target_cell = cell as i8 + 7 * direction;
    let mut is_empty: bool = is_cell_empty(board, target_cell as u8);

    let mut target_rank: i8 = target_cell / 8;
    let mut is_on_next_rank: bool = rank == (target_rank - direction) as u8;

    if is_valid_cell_and_not_same_team(board, target_cell as u8, is_white) && !is_empty && is_on_next_rank {
        let is_cell_on_last_rank = if is_white { target_rank == 7 } else { target_rank == 0 };
        legal_moves[move_n] = generate_move(cell, target_cell as u8, false, false, is_cell_on_last_rank, true);
        move_n += 1;
    }

    // Right top square checking
    target_cell = cell as i8 + 9 * direction;
    is_empty = is_cell_empty(board, target_cell as u8);

    target_rank = target_cell / 8;
    is_on_next_rank = rank == (target_rank - direction) as u8;

    if is_valid_cell_and_not_same_team(board, target_cell as u8, is_white) && !is_empty && is_on_next_rank {
        let is_cell_on_last_rank: bool = if is_white { target_rank == 7 } else { target_rank == 0 };
        legal_moves[move_n] = generate_move(cell, target_cell as u8, false, false, is_cell_on_last_rank, true);
        move_n += 1;
    }

    if last_move & 8 == 8 {
        try_insert_pawn_enpassant_move(board, &mut legal_moves, last_move, cell, move_n);
    }

    result_array.copy_from_slice(&legal_moves);
}

fn get_rook_legal_moves(board: &mut Board, result_array: &mut [u16], cell: u8) {
    let mut legal_moves: Vec<u16> = vec![0; 27];
    let is_attacker_white: bool = is_piece_on_cell_white(board, cell);

    let file: u8 = cell % 8;
    let rank: u8 = cell / 8;
    let mut move_n: usize = 0;

    // LOGIC -> loop through all 4 directions (top, bottom, left, right)
    // and keep adding to legal_moves until reaches obstacle
    // UP Loop
    for top in (rank + 1)..8 {
        let target_cell: u8 = file + top * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }
    }

    // DOWN loop
    for bottom in (0..rank).rev() {
        let target_cell: u8 = file + bottom * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);

        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }
    }

    // LEFT loop
    for left in (0..file).rev() {
        let target_cell: u8 = left + rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }
    }

    // RIGHT loop
    for right in (file + 1)..8 {
        let target_cell: u8 = right + rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            break;
        } else {
            break;
        }
    }

    result_array.copy_from_slice(&legal_moves);
}

fn get_bishop_legal_moves(board: &mut Board, result_array: &mut [u16], cell: u8) {
    let mut legal_moves: Vec<u16> = vec![0; 27];

    let is_attacker_white: bool = is_piece_on_cell_white(board, cell);
    let file: u8 = cell % 8;
    let rank: u8 = cell / 8;
    let mut move_n: usize = 0;

    // LOGIC -> Almost exactly the same as the rook function (above this function),
    // However, instead of going in 4 directions (up, down, left, right), we do the
    // 4 diagonals.
    // ORDER -> right top, right bottom, left top, left bottom
    // RIGHT TOP LOOP
    let mut diag = 1;
    while file + diag < 8 && rank + diag < 8 {
        let new_file: u8 = file + diag;
        let new_rank: u8 = rank + diag;

        let target_cell: u8 = new_file + new_rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }
        diag += 1;
    }

    // RIGHT BOTTOM LOOP
    diag = 1;
    while file + diag < 8 {
        let new_file: u8 = file + diag;
        let new_rank: u8 = rank - diag;

        let target_cell: u8 = new_file + new_rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }

        diag += 1;
    }

    // LEFT TOP LOOP
    diag = 1;
    while rank + diag < 8 {
        let new_file: u8 = file - diag;
        let new_rank: u8 = rank + diag;

        let target_cell: u8 = new_file + new_rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
            move_n += 1;
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            move_n += 1;
            break;
        } else {
            break;
        }

        diag += 1;
    }

    // LEFT BOTTOM LOOP
    diag = 1;
    while file - diag < 8 && rank - diag < 8 {
        let new_file: u8 = file - diag;
        let new_rank: u8 = rank - diag;

        let target_cell: u8 = new_file + new_rank * 8;

        let is_white: bool = is_piece_on_cell_white(board, target_cell);
        let is_empty: bool = is_cell_empty(board, target_cell);
        let is_legal: bool = is_valid_cell_and_not_same_team(board, target_cell, is_attacker_white);

        if is_empty && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, false);
        } else if is_white != is_attacker_white && is_legal {
            legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, true);
            break;
        } else {
            break;
        }

        diag += 1;
    }

    result_array.copy_from_slice(&legal_moves);
}

fn get_knight_legal_moves(board: &mut Board, result_array: &mut [u16], cell: u8) {
    let mut legal_moves: Vec<u16> = vec![0; 27];

    let is_white: bool = is_piece_on_cell_white(board, cell);
    let attacker_file: u8 = cell % 8;
    let attacker_rank: u8 = cell / 8;

    // It is hard to loop through all the legal moves of a knight with a simple
    // for loop, so instead, I am going to hardcode them, then check one by one whether they stand.
    let mut potential_moves: Vec<u8> = Vec::new();
    if attacker_rank < 6 {
        if attacker_file < 7 {
            potential_moves.push(cell + 17);
        }
        if attacker_file > 0 {
            potential_moves.push(cell + 15);
        }
    }

    if attacker_file < 6 {
        if attacker_rank < 7 {
            potential_moves.push(cell + 10);
        }
        if attacker_rank > 0 {
            potential_moves.push(cell - 6);
        }
    }

    if attacker_rank > 1 {
        if attacker_file < 7 {
            potential_moves.push(cell - 15);
        }
        if attacker_file > 0 {
            potential_moves.push(cell - 17);
        }
    }

    if attacker_file > 1 {
        if attacker_rank < 7 {
            potential_moves.push(cell - 10);
        }
        if attacker_rank > 0 {
            potential_moves.push(cell + 6);
        }
    }

    let mut move_n: usize = 0;
    for i in 0..27 {
        if potential_moves.len() >= i {
            break;
        }

        let target_cell: u8 = potential_moves[i];
        if !is_valid_cell_and_not_same_team(board, target_cell, is_white) {
            continue;
        }

        let file: u8 = target_cell % 8;
        let rank: u8 = target_cell / 8;

        if file == attacker_file || rank == attacker_rank {
            continue;
        }

        let is_capturing: bool = !is_cell_empty(board, target_cell);
        legal_moves[move_n] = generate_move(cell, target_cell, false, false, false, is_capturing);
        move_n += 1;
    }

    result_array.copy_from_slice(&legal_moves);
}

fn get_queen_legal_moves(board: &mut Board, result_array: &mut [u16], cell: u8) {
    // A very clever trick : A king's moves are just the combination of a rook's and a bishop's.
    // So that's what we're doing, just calling both the get_rook_legal_moves and the get_bishop_legal_moves
    // functions, then combining the result.

    let mut legal_moves: Vec<u16> = Vec::new();
    let mut rook_legal_moves: Vec<u16> = Vec::new();
    let mut bishop_legal_moves: Vec<u16> = Vec::new();

    get_rook_legal_moves(board, &mut rook_legal_moves, cell);
    get_bishop_legal_moves(board, &mut bishop_legal_moves, cell);

    let mut last = 0;
    for i in 0..27 {
        if rook_legal_moves[i] == 0 {
            break;
        }

        legal_moves[i] = rook_legal_moves[i];
        last = i + 1;
    }

    for i in last..27 {
        if bishop_legal_moves[i - last - 1] == 0 {
            break;
        }

        legal_moves[i] = bishop_legal_moves[i - last - 1];
    }

    result_array.copy_from_slice(&legal_moves);
}

fn get_king_legal_moves(board: &mut Board, result_array: &mut [u16], cell: u8, castle_flags: u8) {
    // We will make an offset array, then loop through all those offsets
    // to get the king's neighbors since a king's legal moves are just 1 cell in all the
    // directions around it.
    let mut legal_moves: Vec<u16> = Vec::new();

    let neighbors: Vec<i8> = vec![7, 8, 9, -1, 1, -9, -8, -7];
    let is_attacker_white: bool = is_piece_on_cell_white(board, cell);

    let mut move_n: usize = 0;
    for offset in neighbors {
        let neighbor: i8 = cell as i8 + offset;

        let file: i8 = neighbor % 8;
        let rank: i8 = neighbor / 8;

        let file_diff: i8 = file - (cell % 8) as i8;
        let rank_diff: i8 = rank - (cell / 8) as i8;

        // Make sure absolute difference is always less or equal to 1, since if a king
        // is beside the edge of the board, the moves can wrap around to the other side, which we don't want.
        let is_file_cut: bool = !(-1..1).contains(&file_diff);
        let is_rank_cut: bool = !(-1..1).contains(&rank_diff);

        if is_valid_cell_and_not_same_team(board, neighbor as u8, is_attacker_white) && !is_file_cut && !is_rank_cut {
            let is_capture: bool = !is_cell_empty(board, neighbor as u8);
            legal_moves[move_n] = generate_move(cell, neighbor as u8, false, false, false, is_capture);
            move_n += 1;
        }
    }

    if is_attacker_white {
        if castle_flags & 1 == 1 {
            let mut blocked_path: bool = false;
            for check_cell in 1..(cell - 1) {
                if is_cell_empty(board, check_cell) {
                    continue;
                }

                blocked_path = true;
                break;
            }

            if !blocked_path {
                legal_moves[move_n] = generate_move(cell, cell - 2, false, true, false, false);
                move_n += 1;
            }
        }

        if castle_flags & 2 == 2 {
            let mut blocked_path: bool = false;
            for check_cell in (cell + 1)..7 {
                if is_cell_empty(board, check_cell) {
                    continue;
                }
                blocked_path = true;
                break;
            }

            if !blocked_path {
                legal_moves[move_n] = generate_move(cell, cell + 2, false, true, false, false);
            }
        }
    } else {
        if castle_flags & 4 == 4 {
            let mut blocked_path: bool = false;
            for check_cell in 56..cell {
                if is_cell_empty(board, check_cell) {
                    continue;
                }
                blocked_path = true;
                break;
            }

            if !blocked_path {
                legal_moves[move_n] = generate_move(cell, cell - 2, false, true, false, false);
                move_n += 1;
            }
        }

        if castle_flags & 8 == 8 {
            let mut blocked_path: bool = false;
            for check_cell in (cell + 1)..64 {
                if is_cell_empty(board, check_cell) {
                    continue;
                }

                blocked_path = true;
                break;
            }

            if !blocked_path {
                legal_moves[move_n] = generate_move(cell, cell + 2, false, true, false, false);
            }
        }
    }

    result_array.copy_from_slice(&legal_moves);
}

fn remove_piece_on_cell(board: &mut Board, cell: u8) {
    let piece_type: *mut u64 = get_piece_type_on_cell(board, cell);

    if piece_type.is_null() {
        return;
    }
    unsafe {
        *piece_type ^= 1u64 << cell;
    }
}

fn try_insert_pawn_enpassant_move(board: &mut Board, legal_moves: &mut [u16], last_move: u16, cell: u8, move_n: usize) {
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn.
    let is_white: bool = is_piece_on_cell_white(board, cell);
    let rank: u8 = cell / 8;

    let left_cell: u8 = if is_white { cell - 1 } else { cell + 1 };
    let right_cell: u8 = if is_white { cell + 1 } else { cell - 1 };

    let left_cell_rank: u8 = left_cell / 8;
    let right_cell_rank: u8 = right_cell / 8;

    let destination_left: u8 = if is_white { cell + 7 } else { cell - 7 };
    let destination_right: u8 = if is_white { cell + 9 } else { cell - 9 };

    let last_destination: u8 = ((last_move >> 4) & 64) as u8;
    let is_left_cell_last_moved: bool = left_cell == last_destination;
    let is_right_cell_last_moved: bool = right_cell == last_destination;

    let is_left_cell_legal = is_valid_cell_and_not_same_team(board, left_cell, is_white) && rank == left_cell_rank;
    let is_right_cell_legal = is_valid_cell_and_not_same_team(board, right_cell, is_white) && rank == right_cell_rank;

    if is_left_cell_last_moved && is_left_cell_legal {
        legal_moves[move_n] = generate_move(cell, destination_left, true, false, false, true);
    } else if is_right_cell_last_moved && is_right_cell_legal {
        legal_moves[move_n] = generate_move(cell, destination_right, true, false, false, true);
    }
}

fn is_valid_cell_and_not_same_team(board: &mut Board, cell: u8, attacker_team: bool) -> bool {
    if !(0..64).contains(&cell) {
        return false;
    }

    is_piece_on_cell_white(board, cell) != attacker_team
}

fn generate_move(
    origin: u8,
    destination: u8,
    is_en_passant: bool,
    is_castle: bool,
    is_promotion: bool,
    is_capture: bool,
) -> u16 {
    let mut move_data: u16;

    move_data = (origin as u16) << 10;
    move_data ^= (destination << 4) as u16;
    move_data ^= (is_en_passant as u16) << 3;
    move_data ^= (is_castle as u16) << 2;
    move_data ^= (is_promotion as u16) << 1;
    move_data ^= is_capture as u16;

    move_data
}
