#include "game.h"

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

    wp = LoadTextureFromImage(LoadImage(R"(../images/wp.png)"));
    wr = LoadTextureFromImage(LoadImage(R"(../images/wr.png)"));
    wn = LoadTextureFromImage(LoadImage(R"(../images/wn.png)"));
    wb = LoadTextureFromImage(LoadImage(R"(../images/wb.png)"));
    wq = LoadTextureFromImage(LoadImage(R"(../images/wq.png)"));
    wk = LoadTextureFromImage(LoadImage(R"(../images/wk.png)"));

    bp = LoadTextureFromImage(LoadImage(R"(../images/bp.png)"));
    br = LoadTextureFromImage(LoadImage(R"(../images/br.png)"));
    bn = LoadTextureFromImage(LoadImage(R"(../images/bn.png)"));
    bb = LoadTextureFromImage(LoadImage(R"(../images/bb.png)"));
    bq = LoadTextureFromImage(LoadImage(R"(../images/bq.png)"));
    bk = LoadTextureFromImage(LoadImage(R"(../images/bk.png)"));
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

void Game::draw_pieces() {
    for (uint8_t i = 0; i < 64; i++) {
        uint8_t x = (i % 8) * CELL_SIZE;
        uint8_t y = SCREEN_HEIGHT - (i / 8 + 1) * CELL_SIZE;
        DrawTexture(get_piece_texture_on_cell(i), x, y, WHITE);
    }
}

std::vector<uint8_t> Game::get_pawn_legal_moves(uint8_t i) {
    std::vector<uint8_t> moves;

    if (!get_piece_texture_on_cell(i + 8).id) moves.push_back(i + 8);

    if (i >= 8 && i <= 15) {
        if (!get_piece_texture_on_cell(i + 16).id) moves.push_back(i + 16);
    }

    return moves;
}

void Game::draw_game() {
    draw_grid();
    draw_pieces();
}
