#pragma once

#include <cstdint>

// This implementation is using bitboards. More on bitboards : https://en.wikipedia.org/wiki/Bitboard
struct Board {
    uint64_t white_pawns{65280ull};
    uint64_t white_rooks{129ull};
    uint64_t white_knights{66ull};
    uint64_t white_bishops{36ull};
    uint64_t white_queens{8ull};
    uint64_t white_king{16ull};

    uint64_t black_pawns{71776119061217280ull};
    uint64_t black_rooks{9295429630892703744ull};
    uint64_t black_knights{4755801206503243776ull};
    uint64_t black_bishops{2594073385365405696ull};
    uint64_t black_queens{576460752303423488ull};
    uint64_t black_king{1152921504606846976ull};
};

extern uint8_t last_placed_cell;

extern bool is_en_passant_available;
extern bool is_white_turn;

extern bool white_in_check;
extern bool black_in_check;

extern bool is_white_long_castle_available;
extern bool is_white_short_castle_available;

extern bool is_black_long_castle_available;
extern bool is_black_short_castle_available;

extern "C" {
bool is_in_check(Board board, bool is_checking_white);

void get_piece_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell);

void handle_move(Board &board, uint64_t *piece_type, uint16_t move);
void get_strictly_legal_moves(Board &board, uint16_t *array_ptr, uint64_t *piece_type);
bool try_make_move_and_check_if_causes_check(Board &board, uint64_t* piece_type, uint16_t move);

void get_pawn_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell); 
void get_rook_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell);
void get_bishop_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell); 
void get_knight_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell);
void get_queen_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell);
void get_king_legal_moves(Board board, uint16_t *array_ptr, uint8_t cell);

void try_insert_pawn_enpassant_move(Board board, uint16_t (&legal_moves)[27], uint8_t cell, int moveN);
void make_move(Board &board, uint64_t* piece_type, uint16_t move);

void remove_piece_on_cell(Board &board, uint8_t cell); 
void handle_promotion(Board &board, uint64_t *chosen_promotion_type);

void get_board(Board &board_to_return);

bool is_cell_empty(Board board, uint8_t cell);
bool is_valid_target(Board board, uint8_t cell, bool is_attacker_white); 
bool is_piece_on_cell_white(Board board, uint8_t cell);

uint8_t get_promotion_pawn_cell(Board board);
uint16_t get_move_from_destination_in_legal_moves(uint16_t *array_ptr, uint8_t destination_to_check);
uint16_t generate_move(uint8_t original_pos, uint8_t destination, bool is_en_passant, bool is_castle, bool is_promotion,
                       bool is_capture); 

uint64_t* get_piece_type_on_cell(Board& board, uint8_t cell);
}
