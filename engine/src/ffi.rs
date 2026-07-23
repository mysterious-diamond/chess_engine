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
    pub fn get_board(board_to_return: Board);

    pub fn handle_move(piece_type: *const u64, move_data: u16);
    pub fn make_move(piece_type: *mut u64, move_data: u16);
    pub fn get_piece_type_on_cell(cell: u8) -> *mut u64;

    pub fn get_piece_legal_moves(array_ptr_to_insert: *const u16, cell: u8);
    pub fn get_strictly_legal_moves(array_ptr: *const u16, piece_type: *mut u64);
}
