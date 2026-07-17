#pragma once

#include <cstdint>

// This implementation is using bitboards. More on bitboards : https://en.wikipedia.org/wiki/Bitboard
struct Board {
    uint64_t whitePawns{65280ull};
    uint64_t whiteRooks{129ull};
    uint64_t whiteKnights{66ull};
    uint64_t whiteBishops{36ull};
    uint64_t whiteQueens{8ull};
    uint64_t whiteKing{16ull};

    uint64_t blackPawns{71776119061217280ull};
    uint64_t blackRooks{9295429630892703744ull};
    uint64_t blackKnights{4755801206503243776ull};
    uint64_t blackBishops{2594073385365405696ull};
    uint64_t blackQueens{576460752303423488ull};
    uint64_t blackKing{1152921504606846976ull};
};

extern Board board;
extern uint8_t last_placed_cell;

extern bool is_en_passant_available;
extern bool white_in_check;
extern bool black_in_check;

extern bool is_white_long_castle_available;
extern bool is_white_short_castle_available;

extern bool is_black_long_castle_available;
extern bool is_black_short_castle_available;

extern "C" {

uint64_t* get_piece_type_on_cell(uint8_t cell);
bool is_piece_on_cell_white(uint8_t cell); 
bool is_cell_empty(uint8_t cell);

bool is_valid_target(uint8_t cell, bool is_attacker_white); 
uint16_t generate_move(uint8_t original_pos, uint8_t destination, bool is_en_passant, bool is_castle, bool is_promotion,
                       bool is_capture); 

void get_pawn_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell); 
void get_rook_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell);
void get_bishop_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell); 
void get_knight_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell);
void get_queen_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell);
void get_king_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell);
void get_piece_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell);

void make_move(uint64_t* piece_type, uint16_t move);
void remove_piece_on_cell(uint8_t cell);

// Some moves might cause the friendly team to be in check, however of course,
// this makes the move illegal. This function is to filter out those moves
// This function also checks castle rights
void get_strictly_legal_moves(uint16_t (*legal_moves)[27]);

bool is_in_check(bool is_checking_white);
bool try_make_move_and_check_if_causes_check(uint64_t* piece_type, uint16_t move); 
}
