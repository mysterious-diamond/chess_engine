#[repr(C)]
pub struct Board {
    white_pawns: u64,
    white_rooks: u64,
    white_knights: u64,
    white_bishops: u64,
    white_queens: u64,
    white_king: u64,

    black_pawns: u64,
    black_rooks: u64,
    black_knights: u64,
    black_bishops: u64,
    black_queens: u64,
    black_king: u64,
}

unsafe extern "C" {
    pub fn get_piece_type_on_cell(cell: u8) -> *mut u64;
    pub fn get_piece_legal_moves(legal_moves: *mut vec![u16; 27]);
}
