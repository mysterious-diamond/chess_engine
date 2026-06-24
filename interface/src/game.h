#pragma once

#include <cstdint>
#include <vector>
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
    bool isPlacementMode = false;

    Color whiteCell = {238, 238, 210, 255};
    Color greenCell = {118, 150, 86, 255};

    // using shorthands (e.g. wp = White Pawn, exceptions are Knights which
    // are wn and bn)
    Texture2D wp, bp, wn, bn, wb, bb, wr, br, wq, bq, wk, bk;

    uint8_t selectedCell;
    std::vector<uint8_t> last_checked_legal_moves;

    void draw_grid();
    void draw_pieces();
    void move_piece(uint8_t cell, uint8_t destination, uint64_t &piece_type);

    Texture2D get_piece_texture_on_cell(uint8_t i);
    void remove_piece_on_cell(uint8_t i);
    void remove_piece_on_cell_with_type(uint8_t i, uint64_t &piece_type);

    bool is_piece_white(uint8_t cell);
    std::vector<uint8_t> get_pawn_legal_moves(uint8_t i);

    void draw_legal_moves(); 

    void reset_placement_variables();

    void handle_white_turn(uint8_t cell, Texture2D selectedCellType);
    void handle_black_turn(uint8_t cell, Texture2D selectedCellType);
    void handle_turn(uint8_t cell, Texture2D selectedCellType);

    void handle_input();
 };
