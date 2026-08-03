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

unsafe extern "C" {
    pub fn try_promote_pawn(board: *mut Board, chosen_piece_type: u8) -> bool;
    pub fn make_move(board: *mut Board, last_move: u16, move_data: u16);
    pub fn get_piece_type_on_cell(board: *mut Board, cell: u8) -> *mut u64;

    pub fn get_piece_legal_moves(board: *mut Board, array_ptr: *mut u16, last_move: u16, cell: u8, castle_flags: u8);
    pub fn get_strictly_legal_moves(board: *mut Board, last_move: u16, array_ptr: *mut u16, piece_type: *mut u64);

    pub fn is_cell_empty(board: *mut Board, cell: u8) -> bool;
    pub fn is_piece_on_cell_white(board: *mut Board, cell: u8) -> bool;
}
