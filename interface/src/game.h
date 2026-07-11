#pragma once

#include <cstdint>
#include <unordered_map>
#include "raylib.h"

#define SCREEN_HEIGHT 800
#define SCREEN_WIDTH 800
#define CELL_SIZE 100
#define CELLS_IN_ROW 8

class Game {
   public:
    uint64_t whitePawns, blackPawns;
    uint64_t whiteKnights, blackKnights;
    uint64_t whiteBishops, blackBishops;
    uint64_t whiteRooks, blackRooks;
    uint64_t whiteQueen, blackQueen;
    uint64_t whiteKing, blackKing;

    Game();
    void step_game();

   private:
    bool isCheck = false;
    bool isWhiteTurn = true;
    bool isEnPassantAvailable = false;

    bool isWhiteShortCastleAvailable = true;
    bool isWhiteLongCastleAvailable = true;

    bool isBlackShortCastleAvailable = true;
    bool isBlackLongCastleAvailable = true;

    bool isPlacementMode = false;

    Color whiteCell = {238, 238, 210, 255};
    Color greenCell = {118, 150, 86, 255};

    // using shorthands (e.g. wp = White Pawn, exceptions are Knights which
    // are wn and bn)
    Texture2D wp, bp, wn, bn, wb, bb, wr, br, wq, bq, wk, bk;

    uint8_t selectedCell;
    uint8_t lastPlacedCell = 64;
    
    bool white_in_check = false;
    bool black_in_check = false;
    std::unordered_map<uint8_t, uint8_t> last_checked_legal_moves;

    void draw_grid();
    void draw_pieces();

    Texture2D get_piece_texture_on_cell(uint8_t i);
    void remove_piece_on_cell(uint8_t i);
    void remove_piece_on_cell_with_type(uint8_t i, uint64_t &piece_type);

    bool is_piece_white(uint8_t cell);
    bool is_cell_empty(uint8_t cell);
    bool is_valid_target(uint8_t target, bool is_attacker_white);

    bool is_white_in_check();
    bool is_black_in_check();

    std::unordered_map<uint8_t, uint8_t> get_pawn_legal_moves(uint8_t i);
    std::unordered_map<uint8_t, uint8_t> get_rook_legal_moves(uint8_t i);
    std::unordered_map<uint8_t, uint8_t> get_bishop_legal_moves(uint8_t i);
    std::unordered_map<uint8_t, uint8_t> get_knight_legal_moves(uint8_t i);
    std::unordered_map<uint8_t, uint8_t> get_queen_legal_moves(uint8_t i);
    std::unordered_map<uint8_t, uint8_t> get_king_legal_moves(uint8_t i);

    // this function is to check the moves you get from calling any of the functions above,
    // so that no legal moves will be able to put its own team in check.
    std::unordered_map<uint8_t, uint8_t> get_strictly_legal_moves(std::unordered_map<uint8_t, uint8_t> legal_moves, uint8_t position, uint64_t &piece_type);

    // this function is to help the above function in trying a move then checking if the team
    // who made the move is now in check because of that move. Then, in the above function,
    // we would delete that move if this function returns true.
    bool try_move_and_check_if_in_check(uint8_t position, uint8_t target, uint8_t attacking_cell, uint64_t &piece_type);

    void draw_legal_moves(); 

    void reset_placement_variables();
    void place_piece(uint8_t cell, uint8_t attacking_cell, uint64_t &piece_type);
    void move_piece(uint8_t cell, uint8_t attacking_cell, uint8_t destination, uint64_t &piece_type);

    void handle_white_placement(uint8_t cell);
    void handle_black_placement(uint8_t cell);
   
    void handle_white_turn(uint8_t cell, Texture2D selectedCellType);
    void handle_black_turn(uint8_t cell, Texture2D selectedCellType);
    void handle_turn(uint8_t cell, Texture2D selectedCellType);

    void handle_input();
 };
