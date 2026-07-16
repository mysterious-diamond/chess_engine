#include <cstdint>

struct Board {
    uint64_t whitePawns{65280ull};
    uint64_t whiteRooks{129ull};
    uint64_t whiteKnights{66ull};
    uint64_t whiteBishops{36ull};
    uint64_t whiteQueen{8ull};
    uint64_t whiteKing{16ull};

    uint64_t blackPawns{71776119061217280ull};
    uint64_t blackRooks{9295429630892703744ull};
    uint64_t blackKnights{4755801206503243776ull};
    uint64_t blackBishops{2594073385365405696ull};
    uint64_t blackQueen{576460752303423488ull};
    uint64_t blackKing{1152921504606846976ull};
};

Board board;

extern "C" {

uint64_t* get_piece_type_on_cell(uint8_t cell) {
    if (board.whitePawns & (1ull << cell)) return &board.whitePawns;
    if (board.whiteRooks & (1ull << cell)) return &board.whiteRooks;
    if (board.whiteKnights & (1ull << cell)) return &board.whiteKnights;
    if (board.whiteBishops & (1ull << cell)) return &board.whiteBishops;
    if (board.whiteQueen & (1ull << cell)) return &board.whiteQueen;
    if (board.whiteKing & (1ull << cell)) return &board.whiteKing;

    if (board.blackPawns & (1ull << cell)) return &board.blackPawns;
    if (board.blackRooks & (1ull << cell)) return &board.blackRooks;
    if (board.blackKnights & (1ull << cell)) return &board.blackKnights;
    if (board.blackBishops & (1ull << cell)) return &board.blackBishops;
    if (board.blackQueen & (1ull << cell)) return &board.blackQueen;
    if (board.blackKing & (1ull << cell)) return &board.blackKing;

    return nullptr;
}

bool is_piece_on_cell_white(uint8_t cell) {
    uint64_t piece_type = *get_piece_type_on_cell(cell);

    if (piece_type == board.whitePawns) return true;
    if (piece_type == board.whiteRooks) return true;
    if (piece_type == board.whiteKnights) return true;
    if (piece_type == board.whiteBishops) return true;
    if (piece_type == board.whiteQueen) return true;
    if (piece_type == board.whiteKing) return true;

    return false;
}

bool is_cell_empty(uint8_t cell) { return !get_piece_type_on_cell(cell); }
bool is_valid_target(uint8_t cell, bool is_attacker_white) {
    if (cell < 0 || cell >= 64) return false;
    if (!is_cell_empty(cell) && is_piece_on_cell_white(cell) == is_attacker_white) return false;
    return true;
}

uint64_t get_pawn_legal_moves(uint8_t cell) {
    uint64_t moves;
    bool is_white = is_piece_on_cell_white(cell);
    int direction = (is_white ? 1 : -1);
    int rank = cell / 8;

    int target_cell = cell + 8 * direction;
    if (is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
        // Advance 1 square
        moves ^= (1ull << target_cell);

        target_cell = cell + 16 * direction;
        bool is_pawn_at_start = (is_white ? rank == 1 : rank == 6);

        // if pawn has never moved before, it can move 2 steps forward
        if (is_pawn_at_start && is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
            moves ^= (1ull << target_cell);
        }
    }

    // Check Left Top Square For Enemy
    target_cell = cell + 7 * direction;
    bool is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves ^= (1ull << target_cell);
    }

    // Check Right Top Square For Enemy
    target_cell = cell + 9 * direction;
    is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        moves ^= (1ull << target_cell);
    }

    // En Passant -- DETAILED BREAKDOWN
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn. Personally got a little confused here.
    if (is_en_passant_available) {
        int left = (is_white ? cell - 1 : cell + 1);
        int right = (is_white ? cell + 1 : cell - 1);

        int leftRank = left / 8;
        int rightRank = right / 8;

        int resulting_left = (is_white ? i + 7 : i - 7);
        int resulting_right = (is_white ? i + 9 : i - 9);

        bool isLeftLastMoved = (left == lastPlacedCell);
        bool isRightLastMoved = (right == lastPlacedCell);

        bool isLegalLeft = is_valid_target(left, is_white) && rank == leftRank;
        bool isLegalRight = is_valid_target(right, is_white) && rank == rightRank;

        if (isLeftLastMoved && isLegalLeft) {
            moves ^= (1ull << resulting_left);
        } else if (isRightLastMoved && isLegalRight) {
            moves ^= (1ull << resulting_right);
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

void remove_piece_on_cell(uint8_t cell) {
    uint64_t* piece_type = get_piece_type_on_cell(cell);

    if (!piece_type) return;
    *piece_type ^= (1ull << cell);
}

void make_move(uint64_t* piece_type, uint8_t original_pos, uint8_t new_pos, uint8_t attack_pos) {
    remove_piece_on_cell(attack_pos);

    *piece_type ^= (1ull << original_pos);
    *piece_type ^= (1ull << new_pos);
}

void try_make_move(uint64_t* piece_type, uint8_t original_pos, uint8_t new_pos) {}
}
