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
    void draw_game();
    void handle_input();

   private:
    bool isEnPassantAvailable = false;
    Color whiteCell = {238, 238, 210, 255};
    Color greenCell = {118, 150, 86, 255};

    Texture2D wp, bp, wn, bn, wb, bb, wr, br, wq, bq, wk, bk;
    // using shorthands (e.g. wp = White Pawn, exceptions are Knights which
    // are wn and bn)

    void draw_grid();
    void draw_pieces();
    Texture2D get_piece_texture_on_cell(uint8_t i);

    std::vector<uint8_t> get_pawn_legal_moves(uint8_t i);
};
