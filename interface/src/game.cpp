#include "game.h"

#include <cstdint>
#include <unordered_map>

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

uint64_t* Game::get_piece_type_from_texture(Texture2D texture) {
    if (texture.id == wp.id)
        return &whitePawns;
    else if (texture.id == wr.id)
        return &whiteRooks;
    else if (texture.id == wn.id)
        return &whiteKnights;
    else if (texture.id == wb.id)
        return &whiteBishops;
    else if (texture.id == wq.id)
        return &whiteQueen;
    else if (texture.id == wk.id)
        return &whiteKing;
    else if (texture.id == bp.id)
        return &blackPawns;
    else if (texture.id == br.id)
        return &blackRooks;
    else if (texture.id == bn.id)
        return &blackKnights;
    else if (texture.id == bb.id)
        return &blackBishops;
    else if (texture.id == bq.id)
        return &blackQueen;
    else if (texture.id == bk.id)
        return &blackKing;

    return nullptr;
}

void Game::remove_piece_on_cell_with_type(uint8_t cell, uint64_t* piece_type) { *piece_type ^= 1ull << cell; }

void Game::remove_piece_on_cell(uint8_t i) {
    if (i == 64) return;

    Texture2D texture = get_piece_texture_on_cell(i);
    if (!texture.id) return;

    uint64_t* type = get_piece_type_from_texture(get_piece_texture_on_cell(i));
    remove_piece_on_cell_with_type(i, type);
}

bool Game::is_valid_target(uint8_t target, bool is_attacker_white) {
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
    if (is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
        // Advance 1 square
        moves[target_cell] = target_cell;

        target_cell = i + 16 * direction;
        bool is_pawn_at_start = (is_white ? rank == 1 : rank == 6);

        // if pawn has never moved before, it can move 2 steps forward
        if (is_pawn_at_start && is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
            moves[target_cell] = target_cell;
        }
    }

    // Check Left Top Square For Enemy
    target_cell = i + 7 * direction;
    bool is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves[target_cell] = target_cell;
    }

    // Check Right Top Square For Enemy
    target_cell = i + 9 * direction;
    is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves[target_cell] = target_cell;
    }

    // En Passant -- DETAILED BREAKDOWN
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn. Personally got a little confused here.
    if (is_en_passant_available) {
        int left = (is_white ? i - 1 : i + 1);
        int right = (is_white ? i + 1 : i - 1);

        int leftRank = left / 8;
        int rightRank = right / 8;

        int resulting_left = (is_white ? i + 7 : i - 7);
        int resulting_right = (is_white ? i + 9 : i - 9);

        bool isLeftLastMoved = (left == lastPlacedCell);
        bool isRightLastMoved = (right == lastPlacedCell);

        bool isLegalLeft = is_valid_target(left, is_white) && rank == leftRank;
        bool isLegalRight = is_valid_target(right, is_white) && rank == rightRank;

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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        bool is_legal = is_valid_target(cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[cell] = cell;
        } else if (is_white != is_attacker_white && is_legal) {
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
        if (!is_valid_target(cell, is_white)) continue;

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

std::unordered_map<uint8_t, uint8_t> Game::get_king_legal_moves(uint8_t i) {
    // Now for this one, I will make an offset array, then loop through all those offsets
    // To get the king's neighbors since a king's legal moves are just 1 cell in all the
    // Directions around it.
    std::unordered_map<uint8_t, uint8_t> legal_moves;
    int neighbors[8] = {7, 8, 9, -1, 1, -9, -8, -7};
    bool is_attacker_white = is_piece_white(i);

    for (int offset : neighbors) {
        int neighbor = i + offset;

        int file = neighbor % 8;
        int rank = neighbor / 8;

        int file_diff = file - (i % 8);
        int rank_diff = rank - (i / 8);

        // Make sure absolute difference is always less or equal to 1, since if a king
        // is beside the edge of the board, the moves can wrap around to the other side, which we don't want.
        bool is_file_cut = !(1 >= file_diff && file_diff >= -1);
        bool is_rank_cut = !(1 >= rank_diff && rank_diff >= -1);

        if (is_valid_target(neighbor, is_attacker_white) && !is_file_cut && !is_rank_cut) {
            legal_moves[neighbor] = neighbor;
        }
    }

    if (is_attacker_white) {
        if (is_white_long_castle_available) {
            bool blockedPath = false;
            for (int cell = i - 1; cell > 0; cell--) {
                if (!get_piece_texture_on_cell(cell).id) continue;
                blockedPath = true;
            }

            if (!blockedPath) legal_moves[i - 2] = i - 2;
        }

        if (is_white_short_castle_available) {
            bool blockedPath = false;
            for (int cell = i + 1; cell < 7; cell++) {
                if (!get_piece_texture_on_cell(cell).id) continue;
                blockedPath = true;
            }

            if (!blockedPath) legal_moves[i + 2] = i + 2;
        }
    } else {
        if (is_black_long_castle_available) {
            bool blockedPath = false;
            for (int cell = i - 1; cell > 56; cell--) {
                if (!get_piece_texture_on_cell(cell).id) continue;
                blockedPath = true;
            }

            if (!blockedPath) legal_moves[i - 2] = i - 2;
        }

        if (is_black_short_castle_available) {
            bool blockedPath = false;
            for (int cell = i + 1; cell < 63; cell++) {
                if (!get_piece_texture_on_cell(cell).id) continue;
                blockedPath = true;
            }

            if (!blockedPath) legal_moves[i + 2] = i + 2;
        }
    }

    return legal_moves;
}

std::unordered_map<uint8_t, uint8_t> Game::get_piece_legal_moves(uint8_t i) {
    Texture2D texture = get_piece_texture_on_cell(i);

    if (texture.id == wp.id || texture.id == bp.id)
        return get_pawn_legal_moves(i);
    else if (texture.id == wr.id || texture.id == br.id)
        return get_rook_legal_moves(i);
    else if (texture.id == wn.id || texture.id == bn.id)
        return get_knight_legal_moves(i);
    else if (texture.id == wb.id || texture.id == bb.id)
        return get_bishop_legal_moves(i);
    else if (texture.id == wq.id || texture.id == bq.id)
        return get_queen_legal_moves(i);
    else if (texture.id == wk.id || texture.id == bk.id)
        return get_king_legal_moves(i);

    return {};
}

bool Game::try_move_and_check_if_in_check(uint8_t position, uint8_t target, uint8_t attacking_cell, uint64_t* piece_type) {
    Texture2D last_piece = get_piece_texture_on_cell(attacking_cell);
    move_piece(position, attacking_cell, target, piece_type);

    bool did_move_cause_check = false;
    if (is_white_turn && is_white_in_check()) {
        did_move_cause_check = true;
    } else if (!is_white_turn && is_black_in_check()) {
        did_move_cause_check = true;
    }

    move_piece(target, position, position, piece_type);

    if (last_piece.id) {
        uint64_t* type = get_piece_type_from_texture(last_piece);
        *type ^= (1ull << attacking_cell);
    }

    if (did_move_cause_check) return true;
    return false;
}

std::unordered_map<uint8_t, uint8_t> Game::get_strictly_legal_moves(std::unordered_map<uint8_t, uint8_t> legal_moves,
                                                                    uint8_t position) {
    std::unordered_map<uint8_t, uint8_t> strictly_legal_moves;

    Texture2D piece_texture = get_piece_texture_on_cell(position);
    uint64_t* piece_type = get_piece_type_from_texture(piece_texture);

    for (auto move : legal_moves) {
        bool result = try_move_and_check_if_in_check(position, move.first, move.second, piece_type);
        if (!result) strictly_legal_moves[move.first] = move.second;
    }

    return strictly_legal_moves;
}

void Game::move_piece(uint8_t cell, uint8_t cell_to_be_attacked, uint8_t destination, uint64_t* piece_type) {
    remove_piece_on_cell(cell_to_be_attacked);

    *piece_type ^= 1ull << cell;
    *piece_type ^= 1ull << destination;
    lastPlacedCell = destination;
}

void Game::place_piece(uint8_t destination, uint8_t cell_to_be_attacked, uint64_t* piece_type) {
    move_piece(selectedCell, cell_to_be_attacked, destination, piece_type);
    is_white_turn = !is_white_turn;

    // Enables en passant move if a pawn went 16 spaces forward
    bool is_piece_pawn = (*piece_type == whitePawns || *piece_type == blackPawns);
    if (!is_piece_pawn) {
        is_en_passant_available = false;
        return;
    }

    if ((destination == selectedCell + 16 || destination == selectedCell - 16)) {
        is_en_passant_available = true;
    } else if (destination / 8 == 7 || destination / 8 == 0) {
        is_selecting_promotion = true;

        // switch back since its not time for the other team yet
        is_white_turn = !is_white_turn;
    } else {
        is_en_passant_available = false;
    }
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

bool Game::is_white_in_check() {
    uint8_t white_king_cell;
    for (int i = 0; i < 64; i++) {
        if (whiteKing & (1ull << i)) {
            white_king_cell = i;
        }
    }

    for (int i = 0; i < 64; i++) {
        uint64_t shift = (1ull << i);

        if ((blackPawns & shift) && get_pawn_legal_moves(i)[white_king_cell]) return true;
        if ((blackRooks & shift) && get_rook_legal_moves(i)[white_king_cell]) return true;
        if ((blackKnights & shift) && get_knight_legal_moves(i)[white_king_cell]) return true;
        if ((blackBishops & shift) && get_bishop_legal_moves(i)[white_king_cell]) return true;
        if ((blackQueen & shift) && get_queen_legal_moves(i)[white_king_cell]) return true;
    }

    return false;
}

bool Game::is_black_in_check() {
    uint8_t black_king_cell;
    for (int i = 0; i < 64; i++) {
        if (blackKing & (1ull << i)) {
            black_king_cell = i;
        }
    }

    for (int i = 0; i < 64; i++) {
        uint64_t shift = (1ull << i);

        if ((whitePawns & shift) && get_pawn_legal_moves(i)[black_king_cell]) return true;
        if ((whiteRooks & shift) && get_rook_legal_moves(i)[black_king_cell]) return true;
        if ((whiteKnights & shift) && get_knight_legal_moves(i)[black_king_cell]) return true;
        if ((whiteBishops & shift) && get_bishop_legal_moves(i)[black_king_cell]) return true;
        if ((whiteQueen & shift) && get_queen_legal_moves(i)[black_king_cell]) return true;
    }

    return false;
}

void Game::reset_placement_variables() {
    selectedCell = 64;
    is_placement_mode = false;
    last_checked_legal_moves = {};
}

void Game::handle_white_placement(uint8_t clickedCell) {
    Texture2D texture = get_piece_texture_on_cell(selectedCell);
    if (!texture.id) return;

    std::unordered_map<uint8_t, uint8_t> legal_moves = last_checked_legal_moves;

    if (legal_moves.count(clickedCell)) {
        uint64_t* type = get_piece_type_from_texture(texture);
        place_piece(clickedCell, legal_moves[clickedCell], type);

        if (texture.id == wr.id) {
            if (is_white_long_castle_available && selectedCell == 0)
                is_white_long_castle_available = false;
            else if (is_white_short_castle_available && selectedCell == 7)
                is_white_short_castle_available = false;
        }

        if (texture.id == wk.id) {  // Edge case for castling
            if (is_white_long_castle_available && selectedCell - 2 == clickedCell) {
                move_piece(0, selectedCell - 1, selectedCell - 1, &whiteRooks);
            } else if (is_white_short_castle_available && selectedCell + 2 == clickedCell) {
                move_piece(7, selectedCell + 1, selectedCell + 1, &whiteRooks);
            }
            is_white_long_castle_available = is_white_short_castle_available = false;
        }
    }

    reset_placement_variables();
}

void Game::handle_black_placement(uint8_t clickedCell) {
    Texture2D texture = get_piece_texture_on_cell(selectedCell);
    if (!texture.id) return;

    std::unordered_map<uint8_t, uint8_t> legal_moves = last_checked_legal_moves;

    if (legal_moves.count(clickedCell)) {
        uint64_t* type = get_piece_type_from_texture(texture);
        place_piece(clickedCell, legal_moves[clickedCell], type);

        if (texture.id == br.id) {
            if (is_black_long_castle_available && selectedCell == 56)
                is_black_long_castle_available = false;
            else if (is_black_short_castle_available && selectedCell == 63)
                is_black_short_castle_available = false;
        }

        if (texture.id == bk.id) {
            // Edge case for castling
            if (is_black_long_castle_available && selectedCell - 2 == clickedCell) {
                move_piece(56, selectedCell - 1, selectedCell - 1, &blackRooks);
            } else if (is_black_short_castle_available && selectedCell + 2 == clickedCell) {
                move_piece(63, selectedCell + 1, selectedCell + 1, &blackRooks);
            }
            is_black_long_castle_available = is_black_short_castle_available = false;
        }
    }

    reset_placement_variables();
}

void Game::check_castling_legality(std::unordered_map<uint8_t, uint8_t>& legal_moves, bool is_team_in_check, uint8_t kingCell) {
    // if long castle position exists, check if either the current position is in check,
    // or the square in which it skips is in check. If so, the move is illegal and shall
    // be deleted. Same logic goes for second if statement.
    if (legal_moves.count(kingCell - 2)) {
        if (is_team_in_check || !legal_moves.count(kingCell - 1)) {
            legal_moves.erase(kingCell - 2);
        }
    }

    if (legal_moves.count(kingCell + 2)) {
        if (is_team_in_check || !legal_moves.count(kingCell + 1)) {
            legal_moves.erase(kingCell + 2);
        }
    }
}

void Game::handle_white_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    if (is_placement_mode) {
        handle_white_placement(clickedCell);
        return;
    }

    if (!is_piece_white(clickedCell)) return;

    uint64_t* type = get_piece_type_from_texture(selectedCellTexture);

    last_checked_legal_moves = get_piece_legal_moves(clickedCell);
    last_checked_legal_moves = get_strictly_legal_moves(last_checked_legal_moves, clickedCell);

    if (selectedCellTexture.id == wk.id) {
        check_castling_legality(last_checked_legal_moves, white_in_check, clickedCell);
    }

    is_placement_mode = true;
    selectedCell = clickedCell;
}

void Game::handle_black_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    if (is_placement_mode) {
        handle_black_placement(clickedCell);
        return;
    }

    if (is_piece_white(clickedCell)) return;

    uint64_t* type = get_piece_type_from_texture(selectedCellTexture);

    last_checked_legal_moves = get_piece_legal_moves(clickedCell);
    last_checked_legal_moves = get_strictly_legal_moves(last_checked_legal_moves, clickedCell);

    if (selectedCellTexture.id == bk.id) {
        check_castling_legality(last_checked_legal_moves, black_in_check, clickedCell);
    }

    is_placement_mode = true;
    selectedCell = clickedCell;
}

void Game::handle_turn(uint8_t clickedCell, Texture2D selectedCellTexture) {
    white_in_check = is_white_in_check();
    black_in_check = is_black_in_check();

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
            last_checked_legal_moves = {};
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
        if (is_placement_mode && this->selectedCell == i) {
            Vector2 mousePos = GetMousePosition();

            DrawTexture(get_piece_texture_on_cell(i), mousePos.x - CELL_SIZE / 2.0, mousePos.y - CELL_SIZE / 2.0, WHITE);
            continue;
        }

        int x = (i % 8) * CELL_SIZE;
        int y = SCREEN_HEIGHT - (i / 8 + 1) * CELL_SIZE;

        if (!is_white_turn) y = 800 - y - 100;
        DrawTexture(get_piece_texture_on_cell(i), x, y, WHITE);
    }
}

void Game::draw_legal_moves() {
    for (auto move : this->last_checked_legal_moves) {
        uint8_t target_cell = move.first;

        uint8_t file = target_cell % 8;
        uint8_t rank = target_cell / 8;

        int x = file * CELL_SIZE + CELL_SIZE / 2;
        int y = SCREEN_HEIGHT - (rank + 1) * CELL_SIZE + CELL_SIZE / 2;

        if (!is_white_turn) y = 800 - y;

        const Color circle_color = {0, 0, 0, 39};
        DrawCircle(x, y, CELL_SIZE / 2.0 - 15, circle_color);
    }
}

uint8_t Game::get_pawn_promotion_cell() {
    for (int i = 56; i <= 63; i++) {
        if (whitePawns & (1ull << i)) {
            return i;
        }
    }

    for (int i = 0; i <= 7; i++) {
        if (blackPawns & (1ull << i)) {
            return i;
        }
    }

    return -1;
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
    int promotionCell = get_pawn_promotion_cell();
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
    promotionSelectionCells[promotionCell] = &whiteQueen;
    promotionSelectionCells[promotionCell - 8] = &whiteRooks;
    promotionSelectionCells[promotionCell - 16] = &whiteKnights;
    promotionSelectionCells[promotionCell - 24] = &whiteBishops;

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
    promotionSelectionCells[promotionCell] = &blackQueen;
    promotionSelectionCells[promotionCell + 8] = &blackRooks;
    promotionSelectionCells[promotionCell + 16] = &blackKnights;
    promotionSelectionCells[promotionCell + 24] = &blackBishops;

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

        int promotionCell = get_pawn_promotion_cell();
        if (promotionCell == -1) return;

        if (is_white_turn) {
            handle_white_promotion_choice(mouseCell, promotionCell);
        } else {
            handle_black_promotion_choice(mouseCell, promotionCell);
        }
    }
}

void Game::step_game() {
    draw_grid();
    draw_pieces();
    draw_legal_moves();

    if (is_selecting_promotion) {
        render_promotion_board();
        handle_promotion_input();
    } else {
        handle_input();
    }
}
