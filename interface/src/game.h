#pragma once

#include <cstdint>
#include "raylib.h"
#include "../../logic/logic.h"

// define macros
#define SCREEN_HEIGHT 800
#define SCREEN_WIDTH 800
#define CELL_SIZE 100
#define CELLS_IN_ROW 8
#define PIECE_IMAGE_SIZE 100
#define PROMOTION_BOARD_HEIGHT 400
#define PROMOTION_BOARD_WIDTH 100

class Game {
   public:
    Board board;
    Game();

    void step_game();
    double evaluate_game();

   private:
    // VARIABLE DECLARATIONS
    // ---------- Game State Variables ------------
    static bool is_selecting_promotion;
    bool is_placement_mode = false;

    uint8_t selectedCell = 64;
    uint16_t last_checked_legal_moves[27] = {};

    // --------- Game Rendering Variables ---------

    Color whiteCell = {238, 238, 210, 255};
    Color greenCell = {118, 150, 86, 255};

    // using shorthands (e.g. wp = White Pawn, exceptions are Knights which are wn and bn)
    Texture2D wp, bp, wn, bn, wb, bb, wr, br, wq, bq, wk, bk;

    // FUNCTION DECLARATIONS
    // ------------------ General Helper Functions -------------------
    Texture2D get_piece_texture_on_cell(uint8_t i);

    // ------------- White & Black Turn Handling Functions -----------
    void reset_placement_variables();

    void handle_white_placement(uint8_t clickedCell);
    void handle_black_placement(uint8_t clickedCell);   

    void handle_white_turn(uint8_t clickedCell, Texture2D selectedCellTexture);
    void handle_black_turn(uint8_t clickedCell, Texture2D selectedCellTexture);

    void handle_turn(uint8_t clickedCell, Texture2D selectedCellTexture);

    // --------------------- Game Step Functions ---------------------
    void draw_grid();
    void draw_pieces();
    void draw_legal_moves();
    void handle_input(); 

    // ------------------- Promotion UI Functions --------------------
    void render_promotion_choices(int boardX, int boardY, int quadrant);

    void handle_white_promotion_choice(int mouseCell, int promotionCell);
    void handle_black_promotion_choice(int mouseCell, int promotionCell);

    void render_promotion_board();
    void handle_promotion_input();
 };
