#include "game.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "../../logic/logic.h"
#include "raylib.h"

Game::Game() {
    wp = LoadTexture(R"(../images/wp.png)");
    wr = LoadTexture(R"(../images/wr.png)");
    wn = LoadTexture(R"(../images/wn.png)");
    wb = LoadTexture(R"(../images/wb.png)");
    wq = LoadTexture(R"(../images/wq.png)");
    wk = LoadTexture(R"(../images/wk.png)");

    bp = LoadTexture(R"(../images/bp.png)");
    br = LoadTexture(R"(../images/br.png)");
    bn = LoadTexture(R"(../images/bn.png)");
    bb = LoadTexture(R"(../images/bb.png)");
    bq = LoadTexture(R"(../images/bq.png)");
    bk = LoadTexture(R"(../images/bk.png)");

    selectedCell = 64;
}

Texture2D Game::get_piece_texture_on_cell(uint8_t i) {
    if (i == 64) return {0};

    if (board.whitePawns & (1ull << i)) {
        return wp;
    }

    if (board.whiteRooks & (1ull << i)) {
        return wr;
    }

    if (board.whiteKnights & (1ull << i)) {
        return wn;
    }

    if (board.whiteBishops & (1ull << i)) {
        return wb;
    }

    if (board.whiteQueens & (1ull << i)) {
        return wq;
    }

    if (board.whiteKing & (1ull << i)) {
        return wk;
    }

    if (board.blackPawns & (1ull << i)) {
        return bp;
    }

    if (board.blackRooks & (1ull << i)) {
        return br;
    }

    if (board.blackKnights & (1ull << i)) {
        return bn;
    }

    if (board.blackBishops & (1ull << i)) {
        return bb;
    }

    if (board.blackQueens & (1ull << i)) {
        return bq;
    }

    if (board.blackKing & (1ull << i)) {
        return bk;
    }

    return {0};
}

void Game::reset_placement_variables() {
    selectedCell = 64;
    is_placement_mode = false;
    std::memset(last_checked_legal_moves, 0, sizeof(last_checked_legal_moves));
}

void Game::handle_white_placement(uint8_t clickedCell) {
    Texture2D texture = get_piece_texture_on_cell(selectedCell);
    if (!texture.id) return;

    uint16_t legal_moves[27];
    std::memcpy(legal_moves, last_checked_legal_moves, sizeof(legal_moves));

    uint16_t move = get_move_from_destination_in_legal_moves(&legal_moves, clickedCell);
    if (move) {
        uint64_t* piece_type = get_piece_type_on_cell(selectedCell);
        make_move(piece_type, move);

        if (texture.id == wr.id) {
            if (is_white_long_castle_available && selectedCell == 0)
                is_white_long_castle_available = false;
            else if (is_white_short_castle_available && selectedCell == 7)
                is_white_short_castle_available = false;
        }
    }

    reset_placement_variables();
}

void Game::handle_black_placement(uint8_t clickedCell) {
    Texture2D texture = get_piece_texture_on_cell(selectedCell);
    if (!texture.id) return;

    uint16_t legal_moves[27];
    std::memcpy(legal_moves, last_checked_legal_moves, sizeof(legal_moves));

    uint8_t move = get_move_from_destination_in_legal_moves(&legal_moves, clickedCell);
    if (move) {
        uint64_t* piece_type = get_piece_type_on_cell(selectedCell);
        make_move(piece_type, move);

        if (texture.id == br.id) {
            if (is_black_long_castle_available && selectedCell == 56)
                is_black_long_castle_available = false;
            else if (is_black_short_castle_available && selectedCell == 63)
                is_black_short_castle_available = false;
        }
    }

    reset_placement_variables();
}

void Game::handle_white_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    if (is_placement_mode) {
        handle_white_placement(clickedCell);
        return;
    }

    if (!is_piece_on_cell_white(clickedCell)) return;
    uint64_t* type = get_piece_type_on_cell(clickedCell);

    get_piece_legal_moves(&last_checked_legal_moves, clickedCell);
    get_strictly_legal_moves(&last_checked_legal_moves);

    is_placement_mode = true;
    selectedCell = clickedCell;
}

void Game::handle_black_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    if (is_placement_mode) {
        handle_black_placement(clickedCell);
        return;
    }

    if (is_piece_on_cell_white(clickedCell) && !is_cell_empty(clickedCell)) return;

    uint64_t* type = get_piece_type_on_cell(clickedCell);

    get_piece_legal_moves(&last_checked_legal_moves, clickedCell);
    get_strictly_legal_moves(&last_checked_legal_moves);

    is_placement_mode = true;
    selectedCell = clickedCell;
}

void Game::handle_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    if (is_white_turn) {
        handle_white_turn(clickedCell, selectedCellTexture);
    } else {
        handle_black_turn(clickedCell, selectedCellTexture);
    }
}

void Game::handle_input() {
    Vector2 mousePos = GetMousePosition();

    if (mousePos.x < 0 || mousePos.x >= SCREEN_WIDTH || mousePos.y < 0 || mousePos.y >= SCREEN_HEIGHT) {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        uint8_t clickedFile = (int)mousePos.x / CELL_SIZE;
        uint8_t clickedRank = (SCREEN_HEIGHT - (int)mousePos.y) / CELL_SIZE;
        uint8_t clickedCell = clickedFile + clickedRank * 8;

        if (!is_white_turn) clickedCell = clickedFile + (8 - clickedRank - 1) * 8;

        Texture2D cell_type = get_piece_texture_on_cell(clickedCell);

        if (is_placement_mode) {
            handle_turn(clickedCell, cell_type);
            return;
        }

        if (!cell_type.id) {
            std::memset(last_checked_legal_moves, 0, sizeof(last_checked_legal_moves));
            return;
        }

        handle_turn(clickedCell, cell_type);
    }
}

void Game::draw_grid() {
    for (int i = 0; i < CELLS_IN_ROW; i++) {
        for (int j = 0; j < CELLS_IN_ROW; j++) {
            // This is used to alternate the color. It goes from
            // white->green->white to then green->white->green. These color
            // schemes are from chess.com

            Color color;
            if (j & 1 && i & 1)
                color = (is_white_turn ? this->whiteCell : this->greenCell);
            else if (j & 1)
                color = (is_white_turn ? this->greenCell : this->whiteCell);
            else if (i & 1)
                color = (is_white_turn ? this->greenCell : this->whiteCell);
            else
                color = (is_white_turn ? this->whiteCell : this->greenCell);

            DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
}

void Game::draw_pieces() {
    for (uint8_t i = 0; i < 64; i++) {
        Texture2D piece_texture = get_piece_texture_on_cell(i);

        int x = (i % 8) * CELL_SIZE;
        int y = SCREEN_HEIGHT - (i / 8 + 1) * CELL_SIZE;

        if (!is_white_turn) y = 800 - y - 100;

        if (white_in_check && piece_texture.id == wk.id || black_in_check && piece_texture.id == bk.id) {
            Rectangle rect = {(float)x, (float)y, CELL_SIZE, CELL_SIZE};
            DrawRectangleLinesEx(rect, 7, RED);
        }

        if (is_placement_mode && this->selectedCell == i) {
            Vector2 mousePos = GetMousePosition();

            DrawTexture(piece_texture, mousePos.x - CELL_SIZE / 2.0, mousePos.y - CELL_SIZE / 2.0, WHITE);
            continue;
        }

        DrawTexture(piece_texture, x, y, WHITE);
    }
}

void Game::draw_legal_moves() {
    for (auto move : last_checked_legal_moves) {
        uint8_t target_cell = move & (64ull << 4);

        uint8_t file = target_cell % 8;
        uint8_t rank = target_cell / 8;

        int x = file * CELL_SIZE + CELL_SIZE / 2;
        int y = SCREEN_HEIGHT - (rank + 1) * CELL_SIZE + CELL_SIZE / 2;

        if (!is_white_turn) y = 800 - y;

        const Color circle_color = {0, 0, 0, 39};
        DrawCircle(x, y, CELL_SIZE / 2.0 - 15, circle_color);
    }
}

void Game::render_promotion_choices(int boardX, int boardY, int quadrant) {
    // There are 4 types of pieces you can promote to,
    // Queen, rook, knight, and the bishop. The following variables
    // are to equally space the icons for them.

    int queenY = boardY + quadrant * 0;
    int rookY = boardY + quadrant * 1;
    int knightY = boardY + quadrant * 2;
    int bishopY = boardY + quadrant * 3;

    // Draw queen
    Texture2D queenTexture = (is_white_turn ? wq : bq);
    DrawTexture(queenTexture, boardX, queenY, WHITE);

    // Draw rook
    Texture2D rookTexture = (is_white_turn ? wr : br);
    DrawTexture(rookTexture, boardX, rookY, WHITE);

    // Draw knight
    Texture2D knightTexture = (is_white_turn ? wn : bn);
    DrawTexture(knightTexture, boardX, knightY, WHITE);

    // Draw bishop
    Texture2D bishopTexture = (is_white_turn ? wb : bb);
    DrawTexture(bishopTexture, boardX, bishopY, WHITE);

    // Draw the outlines for the icons
    DrawRectangleLines(boardX, queenY, PIECE_IMAGE_SIZE, PIECE_IMAGE_SIZE, BLACK);
    DrawRectangleLines(boardX, rookY, PIECE_IMAGE_SIZE, PIECE_IMAGE_SIZE, BLACK);
    DrawRectangleLines(boardX, knightY, PIECE_IMAGE_SIZE, PIECE_IMAGE_SIZE, BLACK);
    DrawRectangleLines(boardX, bishopY, PIECE_IMAGE_SIZE, PIECE_IMAGE_SIZE, BLACK);
}

void Game::render_promotion_board() {
    int promotionCell = get_promotion_pawn_cell();
    if (promotionCell == -1) return;
    if (!is_white_turn) {
        // if is black turn, reverse the cell position so the menu appears upright.
        // we can also do this by just changing the variables below, but it is simply easier to just change the promotionCell.
        // Do note that the original promotionCell variable is correct, we are simply making things easier.

        int promotionFile = promotionCell % 8;
        int newPromotionRank = 8 - (promotionCell / 8) - 1;

        promotionCell = promotionFile + newPromotionRank * 8;
    }

    // Draw the promotion board
    int x = (promotionCell % 8) * 100;
    int y = (8 - promotionCell / 8 - 1) * 100;

    DrawRectangle(x, y, PROMOTION_BOARD_WIDTH, PROMOTION_BOARD_HEIGHT, WHITE);
    DrawRectangleLines(x, y, PROMOTION_BOARD_WIDTH, PROMOTION_BOARD_HEIGHT, BLACK);

    int quadrant = PROMOTION_BOARD_HEIGHT / 4;
    render_promotion_choices(x, y, quadrant);
}

void Game::handle_white_promotion_choice(int mouseCell, int promotionCell) {
    std::unordered_map<uint8_t, uint64_t*> promotionSelectionCells;
    promotionSelectionCells[promotionCell] = &board.whiteQueens;
    promotionSelectionCells[promotionCell - 8] = &board.whiteRooks;
    promotionSelectionCells[promotionCell - 16] = &board.whiteKnights;
    promotionSelectionCells[promotionCell - 24] = &board.whiteBishops;

    for (auto option : promotionSelectionCells) {
        if (option.first != mouseCell) continue;

        whitePawns ^= (1ull << promotionCell);
        *(option.second) ^= (1ull << promotionCell);
        is_selecting_promotion = false;
        is_white_turn = false;
    }
}

void Game::handle_black_promotion_choice(int mouseCell, int promotionCell) {
    std::unordered_map<uint8_t, uint64_t*> promotionSelectionCells;
    promotionSelectionCells[promotionCell] = &board.blackQueens;
    promotionSelectionCells[promotionCell + 8] = &board.blackRooks;
    promotionSelectionCells[promotionCell + 16] = &board.blackKnights;
    promotionSelectionCells[promotionCell + 24] = &board.blackBishops;

    for (auto option : promotionSelectionCells) {
        if (option.first != mouseCell) continue;

        blackPawns ^= (1ull << promotionCell);
        *(option.second) ^= (1ull << promotionCell);
        is_selecting_promotion = false;
        is_white_turn = true;
    }
}

void Game::handle_promotion_input() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();

        int mouseFile = (int)mousePos.x / 100;
        int mouseRank = 8 - ((int)mousePos.y / 100) - 1;

        if (!is_white_turn) {
            mouseRank = 8 - mouseRank - 1;
        }
        int mouseCell = mouseFile + mouseRank * 8;

        int promotionCell = get_promotion_pawn_cell();
        if (promotionCell == -1) return;

        if (is_white_turn) {
            handle_white_promotion_choice(mouseCell, promotionCell);
        } else {
            handle_black_promotion_choice(mouseCell, promotionCell);
        }
    }
}

void Game::step_game() {
    if (is_selecting_promotion) {
        handle_promotion_input();
    } else {
        handle_input();
    }

    draw_grid();
    draw_pieces();
    draw_legal_moves();

    if (is_selecting_promotion) render_promotion_board();
}

double evaluate_game() { double score = 0; }
