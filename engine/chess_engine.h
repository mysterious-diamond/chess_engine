// engine/chess_engine.h
#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include <cstdint>

struct Board {
    uint64_t white_pawns;
    uint64_t white_rooks;
    uint64_t white_knights;
    uint64_t white_bishops;
    uint64_t white_queens;
    uint64_t white_king;
    uint64_t black_pawns;
    uint64_t black_rooks;
    uint64_t black_knights;
    uint64_t black_bishops;
    uint64_t black_queens;
    uint64_t black_king;
};

extern "C" {
    uint16_t get_engine_move(Board board, uint8_t search_depth, uint16_t last_move, bool is_team_white);

    bool is_in_check(Board *board, bool is_checking_white); 
    void get_piece_legal_moves(Board *board, uint16_t *array_ptr, uint16_t last_move, uint8_t cell, uint8_t castle);
    void get_strictly_legal_moves(Board *board, uint16_t last_move, uint16_t *array_ptr, uint64_t *piece_type); 

    bool try_make_move_and_check_if_causes_check(Board *board, uint64_t *piece_type, uint16_t last_move, uint16_t move_data);
    void make_move(Board *board, uint16_t last_move, uint16_t move_data); 
    uint64_t *get_piece_type_on_cell(Board *board, uint8_t cell); 

    bool try_promote_pawn(Board *board, uint8_t chosen_promotion_type);
    bool is_cell_empty(Board *board, uint8_t cell); 
    bool is_piece_on_cell_white(Board *board, uint8_t cell);

    uint8_t get_promotion_pawn_cell(Board *board);
}

#endif
