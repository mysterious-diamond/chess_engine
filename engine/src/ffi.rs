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
}
