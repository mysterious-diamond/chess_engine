#include "game.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "../../logic/logic.h"
#include "raylib.h"

bool Game::is_selecting_promotion = false;

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
    std::memset(last_checked_legal_moves, 0, sizeof(*last_checked_legal_moves));
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
        if (promotionCell == 64) return;

        if (is_white_turn) {
            handle_white_promotion_choice(mouseCell, promotionCell);
        } else {
            handle_black_promotion_choice(mouseCell, promotionCell);
        }
    }
}

void Game::handle_input() {
    Vector2 mousePos = GetMousePosition();

    if (mousePos.x < 0 || mousePos.x >= SCREEN_WIDTH || mousePos.y < 0 || mousePos.y >= SCREEN_HEIGHT) {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        uint8_t clickedFile = (int)mousePos.x / CELL_SIZE;
        uint8_t clickedRank = 7 - ((int)mousePos.y / CELL_SIZE);

        uint8_t clickedCell = clickedFile + clickedRank * 8;
        if (!is_white_turn) clickedCell = clickedFile + (8 - clickedRank - 1) * 8;

        Texture2D cell_texture = get_piece_texture_on_cell(clickedCell);

        if (is_placement_mode) {
            handle_turn(clickedCell, cell_texture);
            return;
        }

        if (is_cell_empty(clickedCell)) {
            std::memset(last_checked_legal_moves, 0, sizeof(last_checked_legal_moves));
            return;
        }

        handle_turn(clickedCell, cell_texture);
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
    for (uint16_t move : last_checked_legal_moves) {
        if (move == 0) break;

        uint8_t target_cell = (move >> 4) & 63ull;

        uint8_t file = target_cell % 8;
        uint8_t rank = target_cell / 8;

        int x = file * CELL_SIZE + CELL_SIZE / 2;
        int y = SCREEN_HEIGHT - (rank + 1) * CELL_SIZE + CELL_SIZE / 2;

        if (!is_white_turn) y = 800 - y;

        const Color circle_color = {0, 0, 0, 39};
        DrawCircle(x, y, CELL_SIZE / 2.0 - 15, circle_color);
    }
}

void Game::render_promotion_board() {
    int promotionCell = get_promotion_pawn_cell();
    if (promotionCell == 64) return;
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

Texture2D Game::get_piece_texture_on_cell(uint8_t i) {
    if (i == 64) return {0};

    if (board.white_pawns & (1ull << i)) {
        return wp;
    }

    if (board.white_rooks & (1ull << i)) {
        return wr;
    }

    if (board.white_knights & (1ull << i)) {
        return wn;
    }

    if (board.white_bishops & (1ull << i)) {
        return wb;
    }

    if (board.white_queens & (1ull << i)) {
        return wq;
    }

    if (board.white_king & (1ull << i)) {
        return wk;
    }

    if (board.black_pawns & (1ull << i)) {
        return bp;
    }

    if (board.black_rooks & (1ull << i)) {
        return br;
    }

    if (board.black_knights & (1ull << i)) {
        return bn;
    }

    if (board.black_bishops & (1ull << i)) {
        return bb;
    }

    if (board.black_queens & (1ull << i)) {
        return bq;
    }

    if (board.black_king & (1ull << i)) {
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

    uint16_t move = get_move_from_destination_in_legal_moves(&legal_moves[0], clickedCell);
    if (move) {
        uint64_t* piece_type = get_piece_type_on_cell(selectedCell);
        handle_move(piece_type, move);

        if (get_promotion_pawn_cell() != 64) is_selecting_promotion = true;
    }

    reset_placement_variables();
}

void Game::handle_black_placement(uint8_t clickedCell) {
    Texture2D texture = get_piece_texture_on_cell(selectedCell);
    if (!texture.id) return;

    uint16_t legal_moves[27];
    std::memcpy(legal_moves, last_checked_legal_moves, sizeof(legal_moves));

    uint16_t move = get_move_from_destination_in_legal_moves(&legal_moves[0], clickedCell);
    if (move) {
        uint64_t* piece_type = get_piece_type_on_cell(selectedCell);
        handle_move(piece_type, move);

        if (get_promotion_pawn_cell() != 64) is_selecting_promotion = true;
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

    get_piece_legal_moves(&last_checked_legal_moves[0], clickedCell);
    get_strictly_legal_moves(&last_checked_legal_moves[0], type);

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

    get_piece_legal_moves(&last_checked_legal_moves[0], clickedCell);
    get_strictly_legal_moves(&last_checked_legal_moves[0], type);

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

void Game::handle_white_promotion_choice(int mouseCell, int promotionCell) {
    std::unordered_map<uint8_t, uint64_t*> promotionSelectionCells;
    promotionSelectionCells[promotionCell] = &board.white_queens;
    promotionSelectionCells[promotionCell - 8] = &board.white_rooks;
    promotionSelectionCells[promotionCell - 16] = &board.white_knights;
    promotionSelectionCells[promotionCell - 24] = &board.white_bishops;

    for (auto option : promotionSelectionCells) {
        if (option.first != mouseCell) continue;

        handle_promotion(option.second);
        is_selecting_promotion = false;
    }
}

void Game::handle_black_promotion_choice(int mouseCell, int promotionCell) {
    std::unordered_map<uint8_t, uint64_t*> promotionSelectionCells;
    promotionSelectionCells[promotionCell] = &board.black_queens;
    promotionSelectionCells[promotionCell + 8] = &board.black_rooks;
    promotionSelectionCells[promotionCell + 16] = &board.black_knights;
    promotionSelectionCells[promotionCell + 24] = &board.black_bishops;

    for (auto option : promotionSelectionCells) {
        if (option.first != mouseCell) continue;

        handle_promotion(option.second);
        is_selecting_promotion = false;
    }
}

double Game::evaluate_game() { return 0.0; }
