#include "game.h"

#include <cstdint>
// #include <iostream>
#include <unordered_map>

#include "raylib.h"

bool ok() {
    bool ye = false;
    return true;
}

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
            // This is used to alternate the color. It goes from
            // white->green->white to then green->white->green. These color
            // schemes are from chess.com

            Color color;
            if (j & 1 && i & 1)
                color = this->whiteCell;
            else if (j & 1)
                color = this->greenCell;
            else if (i & 1)
                color = this->greenCell;
            else
                color = this->whiteCell;

            DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
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

void Game::remove_piece_on_cell_with_type(uint8_t cell, uint64_t& piece_type) { piece_type ^= 1ull << cell; }

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

            DrawTexture(get_piece_texture_on_cell(i), mousePos.x - CELL_SIZE / 2.0, mousePos.y - CELL_SIZE / 2.0, WHITE);
            continue;
        }

        int x = (i % 8) * CELL_SIZE;
        int y = SCREEN_HEIGHT - (i / 8 + 1) * CELL_SIZE;
        DrawTexture(get_piece_texture_on_cell(i), x, y, WHITE);
    }
}

bool Game::is_legal_attack(uint8_t target, bool is_attacker_white) {
    uint8_t file = target % 8;
    uint8_t rank = target / 8;

    if (file > 7 || file < 0) return false;
    if (rank > 7 || rank < 0) return false;

    bool is_target_white = is_piece_white(target);
    bool is_empty = is_cell_empty(target);
    return is_empty || is_target_white != is_attacker_white;
}

std::unordered_map<uint8_t, uint8_t> Game::get_pawn_legal_moves(uint8_t i) {
    if (i == 64) return {};

    std::unordered_map<uint8_t, uint8_t> moves;
    bool is_white = is_piece_white(i);
    int direction = (is_white ? 1 : -1);
    int rank = i / 8;

    int target_cell = i + 8 * direction;
    if (is_cell_empty(target_cell)) {
        moves[target_cell] = target_cell;

        target_cell = i + 16 * direction;
        if ((is_white ? rank == 1 : rank == 6) && is_cell_empty(target_cell)) {
            moves[target_cell] = target_cell;
        }
    }

    // Check Left Top Square For Enemy
    target_cell = i + 7 * direction;
    bool is_empty = is_cell_empty(target_cell);
    if (is_legal_attack(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves[target_cell] = target_cell;
    }

    // Check Right Top Square For Enemy
    target_cell = i + 9 * direction;
    is_empty = is_cell_empty(target_cell);
    if (is_legal_attack(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves[target_cell] = target_cell;
    }

    // En Passant -- DETAILED BREAKDOWN
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn. Personally got a little confused here.
    if (isEnPassantAvailable) {
        int left = (is_white ? i - 1 : i + 1);
        int right = (is_white ? i + 1 : i - 1);

        int leftRank = left / 8;
        int rightRank = right / 8;

        int resulting_left = (is_white ? i + 7 : i - 7);
        int resulting_right = (is_white ? i + 9 : i - 9);

        bool isLeftLastMoved = (left == lastPlacedCell);
        bool isRightLastMoved = (right == lastPlacedCell);

        bool isLegalLeft = is_legal_attack(left, is_white) && rank == leftRank;
        bool isLegalRight = is_legal_attack(right, is_white) && rank == rightRank;

        if (isLeftLastMoved && isLegalLeft) {
            moves[resulting_left] = left;
        } else if (isRightLastMoved && isLegalRight) {
            moves[resulting_right] = right;
        }
    }

    return moves;
}

std::unordered_map<uint8_t, uint8_t> Game::get_rook_legal_moves(uint8_t i) {
    std::unordered_map<uint8_t, uint8_t> legal_moves;

    bool is_attacker_white = is_piece_white(i);

    int file = i % 8;
    int rank = i / 8;

    // LOGIC -> loop through all 4 directions (up, down, left, right)
    // and keep adding to legal_moves until reaches obstacle
    for (int top = rank + 1; top < 8; top++) {
        int cell = file + top * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    for (int bottom = rank - 1; bottom >= 0; bottom--) {
        int cell = file + bottom * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    for (int left = file - 1; left >= 0; left--) {
        int cell = left + rank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    for (int right = file + 1; right < 8; right++) {
        int cell = right + rank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    return legal_moves;
}

std::unordered_map<uint8_t, uint8_t> Game::get_bishop_legal_moves(uint8_t i) {
    std::unordered_map<uint8_t, uint8_t> legal_moves;

    bool is_attacker_white = is_piece_white(i);
    int file = i % 8;
    int rank = i / 8;

    // LOGIC -> Almost exactly the same as the rook function (above this function),
    // However, instead of going in 4 directions (up, down, left, right), we do the
    // 4 diagonals.
    // ORDER -> right top, right bottom, left top, left bottom
    // RIGHT TOP LOOP
    for (int multi = 1; file + multi < 8 && rank + multi < 8; multi++) {
        int newFile = file + multi;
        int newRank = rank + multi;

        int cell = newFile + newRank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    // RIGHT BOTTOM LOOP
    for (int multi = 1; file + multi < 8 && rank - multi >= 0; multi++) {
        int newFile = file + multi;
        int newRank = rank - multi;

        int cell = newFile + newRank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    // LEFT TOP LOOP
    for (int multi = 1; file - multi >= 0 && rank + multi < 8; multi++) {
        int newFile = file - multi;
        int newRank = rank + multi;

        int cell = newFile + newRank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    // LEFT BOTTOM LOOP
    for (int multi = 1; file - multi >= 0 && rank - multi >= 0; multi++) {
        int newFile = file - multi;
        int newRank = rank - multi;

        int cell = newFile + newRank * 8;

        bool is_white = is_piece_white(cell);
        bool is_empty = is_cell_empty(cell);

        if (is_empty) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white) {
            legal_moves[cell] = cell;
            break;
        } else {
            break;
        }
    }

    return legal_moves;
}

std::unordered_map<uint8_t, uint8_t> Game::get_knight_legal_moves(uint8_t i) {
    std::unordered_map<uint8_t, uint8_t> legal_moves;

    bool is_white = is_piece_white(i);
    uint8_t attacker_file = i % 8;
    uint8_t attacker_rank = i / 8;

    // It is hard to loop through all the legal moves of a knight with a simple
    // for loop, so instead, I am going to hardcode them, then check one by one whether they stand.
    std::unordered_map<uint8_t, uint8_t> potential_moves;
    if (attacker_rank < 6) {
        if (attacker_file < 7) potential_moves[i + 17] = i + 17;
        if (attacker_file > 0) potential_moves[i + 15] = i + 15;
    }

    if (attacker_file < 6) {
        if (attacker_rank < 7) potential_moves[i + 10] = i + 10;
        if (attacker_rank > 0) potential_moves[i - 6] = i - 6;
    }

    if (attacker_rank > 1) {
        if (attacker_file < 7) potential_moves[i - 15] = i - 15;
        if (attacker_file > 0) potential_moves[i - 17] = i - 17;
    }

    if (attacker_file > 1) {
        if (attacker_rank < 7) potential_moves[i - 10] = i - 10;
        if (attacker_rank > 0) potential_moves[i + 6] = i + 6;
    }

    for (auto pair : potential_moves) {
        uint8_t cell = pair.first;
        if (!is_legal_attack(cell, is_white)) continue;

        int file = cell % 8;
        int rank = cell / 8;

        if (file == attacker_file || rank == attacker_rank) continue;
        legal_moves[cell] = cell;
    }

    return legal_moves;
}

std::unordered_map<uint8_t, uint8_t> Game::get_queen_legal_moves(uint8_t i) {
    // A very clever trick : A king's moves are just the combination of a rook's and a bishop's.
    // So that's what we're doing, just calling both the get_rook_legal_moves and the get_bishop_legal_moves
    // functions, then combining the result.
    std::unordered_map<uint8_t, uint8_t> rook_legal_moves = get_rook_legal_moves(i);
    std::unordered_map<uint8_t, uint8_t> bishop_legal_moves = get_bishop_legal_moves(i);

    std::unordered_map<uint8_t, uint8_t> legal_moves = rook_legal_moves;
    for (auto move : bishop_legal_moves) {
        legal_moves[move.first] = move.second;
    }

    return legal_moves;
}

void Game::move_piece(uint8_t cell, uint8_t attacking_cell, uint8_t destination, uint64_t& piece_type) {
    remove_piece_on_cell(attacking_cell);

    piece_type ^= 1ull << cell;
    piece_type ^= 1ull << destination;
    lastPlacedCell = destination;
}

void Game::place_piece(uint8_t cell, uint8_t attacking_cell, uint64_t& piece_type) {
    move_piece(selectedCell, attacking_cell, cell, piece_type);

    // Enables en passant move if a pawn went 16 spaces forward
    bool is_piece_pawn = (piece_type == whitePawns || piece_type == blackPawns);
    if (is_piece_pawn && cell == selectedCell + 16 || cell == selectedCell - 16) {
        isEnPassantAvailable = true;
    } else {
        isEnPassantAvailable = false;
    }

    isWhiteTurn = !isWhiteTurn;
}

bool Game::is_piece_white(uint8_t cell) {
    Texture2D type = get_piece_texture_on_cell(cell);

    // This early return is for empty cells
    if (!type.id) return false;

    if (type.id == wp.id || type.id == wb.id || type.id == wn.id || type.id == wr.id || type.id == wq.id || type.id == wk.id)
        return true;
    else
        return false;
}

bool Game::is_cell_empty(uint8_t cell) {
    Texture2D type = get_piece_texture_on_cell(cell);
    return !type.id;
}

void Game::reset_placement_variables() {
    selectedCell = 64;
    isPlacementMode = false;
    last_checked_legal_moves = {};
}

void Game::handle_white_placement(uint8_t cell) {
    if (get_piece_texture_on_cell(selectedCell).id == wp.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_pawn_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], whitePawns);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == wr.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_rook_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], whiteRooks);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == wn.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_knight_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], whiteKnights);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == wb.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_bishop_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], whiteBishops);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == wq.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_queen_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], whiteQueen);
        }

        reset_placement_variables();
    }
}

void Game::handle_black_placement(uint8_t cell) {
    if (get_piece_texture_on_cell(selectedCell).id == bp.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_pawn_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], blackPawns);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == br.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_rook_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], blackRooks);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == bn.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_knight_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], blackKnights);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == bb.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_bishop_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], blackBishops);
        }

        reset_placement_variables();
    } else if (get_piece_texture_on_cell(selectedCell).id == bq.id) {
        std::unordered_map<uint8_t, uint8_t> legal_moves = get_queen_legal_moves(selectedCell);

        if (legal_moves.count(cell)) {
            place_piece(cell, legal_moves[cell], blackQueen);
        }

        reset_placement_variables();
    }
}

void Game::handle_white_turn(uint8_t cell, Texture2D selectedCellType) {
    if (isPlacementMode) {
        handle_white_placement(cell);
        return;
    }

    if (selectedCellType.id == wp.id) {
        last_checked_legal_moves = get_pawn_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == wr.id) {
        last_checked_legal_moves = get_rook_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == wn.id) {
        last_checked_legal_moves = get_knight_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == wb.id) {
        last_checked_legal_moves = get_bishop_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == wq.id) {
        last_checked_legal_moves = get_queen_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    }
}

void Game::handle_black_turn(uint8_t cell, Texture2D selectedCellType) {
    if (isPlacementMode) {
        handle_black_placement(cell);
        return;
    }

    if (selectedCellType.id == bp.id) {
        this->last_checked_legal_moves = get_pawn_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == br.id) {
        last_checked_legal_moves = get_rook_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == bn.id) {
        last_checked_legal_moves = get_knight_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == bb.id) {
        last_checked_legal_moves = get_bishop_legal_moves(cell);

        isPlacementMode = true;
        selectedCell = cell;
    } else if (selectedCellType.id == bq.id) {
        last_checked_legal_moves = get_queen_legal_moves(cell);

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

    if (mousePos.x < 0 || mousePos.x >= SCREEN_WIDTH || mousePos.y < 0 || mousePos.y >= SCREEN_HEIGHT) {
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
    for (auto move : this->last_checked_legal_moves) {
        uint8_t target_cell = move.first;

        uint8_t file = target_cell % 8;
        uint8_t rank = target_cell / 8;

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
