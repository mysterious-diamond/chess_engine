#include "game.h"

#include <cstdint>
// #include <iostream>
#include <vector>

#include "raylib.h"

uint64_t initializePawns(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        for (int i = 8; i < 16; i++) {
            result += (1ull << i);
        }
    } else {
        for (int i = 55; i > 47; i--) {
            result += (1ull << i);
        }
    }

    return result;
}

uint64_t initializeRooks(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        result += 1;
        result += 1ull << 7;
    } else {
        result += 1ull << 56;
        result += 1ull << 63;
    }

    return result;
}

uint64_t initializeKnights(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        result += 1ull << 1;
        result += 1ull << 6;
    } else {
        result += 1ull << 57;
        result += 1ull << 62;
    }

    return result;
}

uint64_t initializeBishops(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        result += 1ull << 2;
        result += 1ull << 5;
    } else {
        result += 1ull << 58;
        result += 1ull << 61;
    }

    return result;
}

uint64_t initializeQueen(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        result += 1ull << 3;
    } else {
        result += 1ull << 59;
    }

    return result;
}

uint64_t initializeKing(bool isWhite) {
    uint64_t result = 0;
    if (isWhite) {
        result += 1ull << 4;
    } else {
        result += 1ull << 60;
    }

    return result;
}

Game::Game() {
    whitePawns = initializePawns(true);
    whiteRooks = initializeRooks(true);
    whiteKnights = initializeKnights(true);
    whiteBishops = initializeBishops(true);
    whiteQueen = initializeQueen(true);
    whiteKing = initializeKing(true);

    blackPawns = initializePawns(false);
    blackRooks = initializeRooks(false);
    blackKnights = initializeKnights(false);
    blackBishops = initializeBishops(false);
    blackQueen = initializeQueen(false);
    blackKing = initializeKing(false);

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

void Game::draw_grid() {
    for (int i = 0; i < CELLS_IN_ROW; i++) {
        for (int j = 0; j < CELLS_IN_ROW; j++) {
            Color color;
            if (j & 1 && i & 1)
                color = this->whiteCell;
            else if (j & 1)
                color = this->greenCell;
            else if (i & 1)
                color = this->greenCell;
            else
                color = this->whiteCell;

            DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                          color);
        }
    }
}

Texture2D Game::get_piece_texture_on_cell(uint8_t i) {
    if (i == 64) return {0};

    if (this->whitePawns & (1ull << i)) {
        return wp;
    }

    if (this->whiteRooks & (1ull << i)) {
        return wr;
    }

    if (this->whiteKnights & (1ull << i)) {
        return wn;
    }

    if (this->whiteBishops & (1ull << i)) {
        return wb;
    }

    if (this->whiteQueen & (1ull << i)) {
        return wq;
    }

    if (this->whiteKing & (1ull << i)) {
        return wk;
    }

    if (this->blackPawns & (1ull << i)) {
        return bp;
    }

    if (this->blackRooks & (1ull << i)) {
        return br;
    }

    if (this->blackKnights & (1ull << i)) {
        return bn;
    }

    if (this->blackBishops & (1ull << i)) {
        return bb;
    }

    if (this->blackQueen & (1ull << i)) {
        return bq;
    }

    if (this->blackKing & (1ull << i)) {
        return bk;
    }

    return {0};
}

void Game::remove_piece_on_cell_with_type(uint8_t cell, uint64_t& piece_type) {
    piece_type ^= 1ull << cell;
}

void Game::remove_piece_on_cell(uint8_t i) {
    if (i == 64) return;

    if (this->whitePawns & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whitePawns);
    }

    if (this->whiteRooks & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whiteRooks);
    }

    if (this->whiteKnights & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whiteKnights);
    }

    if (this->whiteBishops & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whiteBishops);
    }

    if (this->whiteQueen & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whiteQueen);
    }

    if (this->whiteKing & (1ull << i)) {
        remove_piece_on_cell_with_type(i, whiteKing);
    }

    if (this->blackPawns & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackPawns);
    }

    if (this->blackRooks & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackRooks);
    }

    if (this->blackKnights & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackKnights);
    }

    if (this->blackBishops & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackBishops);
    }

    if (this->blackQueen & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackQueen);
    }

    if (this->blackKing & (1ull << i)) {
        remove_piece_on_cell_with_type(i, blackKing);
    }
}

void Game::draw_pieces() {
    for (uint8_t i = 0; i < 64; i++) {
        if (isPlacementMode && this->selectedCell == i) {
            Vector2 mousePos = GetMousePosition();
            DrawTexture(get_piece_texture_on_cell(i),
                        mousePos.x - CELL_SIZE / 2.0,
                        mousePos.y - CELL_SIZE / 2.0, WHITE);

            continue;
        }

        int x = (i % 8) * CELL_SIZE;
        int y = SCREEN_HEIGHT - (i / 8 + 1) * CELL_SIZE;
        DrawTexture(get_piece_texture_on_cell(i), x, y, WHITE);
    }
}

std::vector<uint8_t> Game::get_pawn_legal_moves(uint8_t i) {
    if (i == 64) return {};

    std::vector<uint8_t> moves;
    bool is_white = is_piece_white(i);
    int reflect = (is_white ? 1 : -1);

    if (!get_piece_texture_on_cell(i + 8 * reflect).id) {
        moves.push_back(i + 8 * reflect);
    }

    if ((is_white && i >= 8 && i <= 15) || (!is_white && i >= 48 && i <= 55)) {
        if (!get_piece_texture_on_cell(i + 16 * reflect).id) {
            moves.push_back(i + 16 * reflect);
        }
    }

    uint8_t file = i % 8;

    if (file != 0 && get_piece_texture_on_cell(i + 7 * reflect).id &&
        is_white != is_piece_white(i + 7 * reflect)) {
        moves.push_back(i + 7 * reflect);
    }

    if (file != 7 && get_piece_texture_on_cell(i + 9 * reflect).id &&
        is_white != is_piece_white(i + 9 * reflect)) {
        moves.push_back(i + 9 * reflect);
    }

    return moves;
}

void Game::move_piece(uint8_t cell, uint8_t destination, uint64_t& piece_type) {
    remove_piece_on_cell(destination);

    piece_type ^= 1ull << cell;
    piece_type ^= 1ull << destination;
}

bool Game::is_piece_white(uint8_t cell) {
    Texture2D type = get_piece_texture_on_cell(cell);

    if (!type.id) return false;

    if (type.id == wp.id || type.id == wb.id || type.id == wn.id ||
        type.id == wr.id || type.id == wq.id || type.id == wk.id)
        return true;
    else
        return false;
}

void Game::reset_placement_variables() {
    selectedCell = 64;
    isPlacementMode = false;
    last_checked_legal_moves = {};
}

void Game::handle_white_turn(uint8_t cell, Texture2D selectedCellType) {
    if (isPlacementMode) {
        if (get_piece_texture_on_cell(selectedCell).id == wp.id) {
            std::vector<uint8_t> legal_moves =
                get_pawn_legal_moves(selectedCell);

            for (uint8_t move : legal_moves) {
                if (move != cell) continue;

                move_piece(selectedCell, cell, whitePawns);
                isWhiteTurn = false;

                break;
            }

            reset_placement_variables();
        }

        return;
    }

    if (selectedCellType.id == wp.id) {
        std::vector<uint8_t> legal_moves = get_pawn_legal_moves(cell);
        this->last_checked_legal_moves = legal_moves;

        isPlacementMode = true;
        selectedCell = cell;
    }
}

void Game::handle_black_turn(uint8_t cell, Texture2D selectedCellType) {
    if (isPlacementMode) {
        if (get_piece_texture_on_cell(selectedCell).id == bp.id) {
            std::vector<uint8_t> legal_moves =
                get_pawn_legal_moves(selectedCell);

            bool moved = false;
            for (uint8_t move : legal_moves) {
                if (move != cell) continue;

                move_piece(selectedCell, cell, blackPawns);
                isWhiteTurn = true;

                break;
            }

            reset_placement_variables();
        }
    }

    if (selectedCellType.id == bp.id) {
        std::vector<uint8_t> legal_moves = get_pawn_legal_moves(cell);
        this->last_checked_legal_moves = legal_moves;

        isPlacementMode = true;
        selectedCell = cell;
    }
}

void Game::handle_turn(uint8_t cell, Texture2D selectedCellType) {
    if (isWhiteTurn) {
        handle_white_turn(cell, selectedCellType);
    } else {
        handle_black_turn(cell, selectedCellType);
    }
}

void Game::handle_input() {
    Vector2 mousePos = GetMousePosition();

    if (mousePos.x < 0 || mousePos.x >= SCREEN_WIDTH || mousePos.y < 0 ||
        mousePos.y >= SCREEN_HEIGHT) {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        uint8_t file = (int)mousePos.x / CELL_SIZE;
        uint8_t rank = (SCREEN_HEIGHT - (int)mousePos.y) / CELL_SIZE;
        uint8_t cell = file + rank * 8;

        Texture2D cell_type = get_piece_texture_on_cell(cell);

        if (isPlacementMode) {
            handle_turn(cell, cell_type);
            return;
        }

        if (!cell_type.id) {
            last_checked_legal_moves = {};
            return;
        }

        handle_turn(cell, cell_type);
    }
}

void Game::draw_legal_moves() {
    for (uint8_t move : this->last_checked_legal_moves) {
        uint8_t file = move % 8;
        uint8_t rank = move / 8;

        int x = file * CELL_SIZE + CELL_SIZE / 2;
        int y = SCREEN_HEIGHT - (rank + 1) * CELL_SIZE + CELL_SIZE / 2;

        const Color circle_color = {0, 0, 0, 39};
        DrawCircle(x, y, CELL_SIZE / 2.0 - 15, circle_color);
    }
}

void Game::step_game() {
    draw_grid();
    draw_pieces();
    handle_input();
    draw_legal_moves();
}
