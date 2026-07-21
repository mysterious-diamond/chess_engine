#include "logic.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_map>

Board board{};
uint8_t last_placed_cell = 64;

bool is_en_passant_available = false;
bool is_white_turn = true;

bool white_in_check = false;
bool black_in_check = false;

bool is_white_long_castle_available = true;
bool is_white_short_castle_available = true;

bool is_black_long_castle_available = true;
bool is_black_short_castle_available = true;

uint64_t* get_piece_type_on_cell(uint8_t cell) {
    if (cell >= 64) return nullptr;

    if (board.whitePawns & (1ull << cell)) return &board.whitePawns;
    if (board.whiteRooks & (1ull << cell)) return &board.whiteRooks;
    if (board.whiteKnights & (1ull << cell)) return &board.whiteKnights;
    if (board.whiteBishops & (1ull << cell)) return &board.whiteBishops;
    if (board.whiteQueens & (1ull << cell)) return &board.whiteQueens;
    if (board.whiteKing & (1ull << cell)) return &board.whiteKing;

    if (board.blackPawns & (1ull << cell)) return &board.blackPawns;
    if (board.blackRooks & (1ull << cell)) return &board.blackRooks;
    if (board.blackKnights & (1ull << cell)) return &board.blackKnights;
    if (board.blackBishops & (1ull << cell)) return &board.blackBishops;
    if (board.blackQueens & (1ull << cell)) return &board.blackQueens;
    if (board.blackKing & (1ull << cell)) return &board.blackKing;

    return nullptr;
}

void handle_promotion(uint64_t* chosen_promotion_type) {
    uint8_t pawn_promotion_cell = get_promotion_pawn_cell();
    if (pawn_promotion_cell == 64) return;

    bool is_white = is_piece_on_cell_white(pawn_promotion_cell);
    uint64_t* piece_type = (is_white ? &board.whitePawns : &board.blackPawns);

    *piece_type ^= (1ull << pawn_promotion_cell);
    *chosen_promotion_type ^= (1ull << pawn_promotion_cell);

    is_white_turn = !is_white_turn;
}

uint8_t get_promotion_pawn_cell() {
    for (int i = 56; i <= 63; i++) {
        if (board.whitePawns & (1ull << i)) {
            return i;
        }
    }

    for (int i = 0; i <= 7; i++) {
        if (board.blackPawns & (1ull << i)) {
            return i;
        }
    }

    return 64;
}

bool is_piece_on_cell_white(uint8_t cell) {
    if (!get_piece_type_on_cell(cell)) return false;
    uint64_t piece_type = *get_piece_type_on_cell(cell);

    if (piece_type == board.whitePawns) return true;
    if (piece_type == board.whiteRooks) return true;
    if (piece_type == board.whiteKnights) return true;
    if (piece_type == board.whiteBishops) return true;
    if (piece_type == board.whiteQueens) return true;
    if (piece_type == board.whiteKing) return true;

    return false;
}

bool is_cell_empty(uint8_t cell) { return !get_piece_type_on_cell(cell); }
bool is_valid_target(uint8_t cell, bool is_attacker_white) {
    if (cell < 0 || cell >= 64) return false;
    if (!is_cell_empty(cell) && is_piece_on_cell_white(cell) == is_attacker_white) return false;
    return true;
}

uint16_t generate_move(uint8_t original_pos, uint8_t destination, bool is_en_passant, bool is_castle, bool is_promotion,
                       bool is_capture) {
    // Encoding Legal Moves :
    // First 6 bits : original position
    // Next 6 bits : destination
    // Last 4 bits : boolean flags
    uint16_t result = 0;
    result ^= (original_pos << 10);
    result ^= (destination << 4);
    result ^= (is_en_passant << 3);
    result ^= (is_castle << 2);
    result ^= (is_promotion << 1);
    result ^= is_capture;

    return result;
}

void get_pawn_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    // Make sure array is set to 0 so it doesnt return garbage values
    std::memset(legal_moves, 0, sizeof(*legal_moves));

    bool is_white = is_piece_on_cell_white(cell);
    int direction = (is_white ? 1 : -1);
    int rank = cell / 8;

    int target_cell = cell + 8 * direction;
    if (is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
        // Advance 1 square
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));

        (*legal_moves)[0] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 0);
        target_cell = cell + 16 * direction;
        bool is_pawn_at_start = (is_white ? rank == 1 : rank == 6);

        // if pawn has never moved before, it can move 2 steps forward
        if (is_pawn_at_start && is_cell_empty(target_cell) && is_valid_target(target_cell, is_white)) {
            (*legal_moves)[1] = generate_move(cell, target_cell, 0, 0, 0, 0);
        }
    }

    // Check Left Top Square For Enemy
    target_cell = cell + 7 * direction;
    bool is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));
        (*legal_moves)[2] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 1);
    }

    // Check Right Top Square For Enemy
    target_cell = cell + 9 * direction;
    is_empty = is_cell_empty(target_cell);
    if (is_valid_target(target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));
        (*legal_moves)[3] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 1);
    }

    // En Passant
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn.
    if (is_en_passant_available) {
        int left = (is_white ? cell - 1 : cell + 1);
        int right = (is_white ? cell + 1 : cell - 1);

        int leftRank = left / 8;
        int rightRank = right / 8;

        int resulting_left = (is_white ? cell + 7 : cell - 7);
        int resulting_right = (is_white ? cell + 9 : cell - 9);

        bool isLeftLastMoved = (left == last_placed_cell);
        bool isRightLastMoved = (right == last_placed_cell);

        bool isLegalLeft = is_valid_target(left, is_white) && rank == leftRank;
        bool isLegalRight = is_valid_target(right, is_white) && rank == rightRank;

        if (isLeftLastMoved && isLegalLeft) {
            (*legal_moves)[4] = generate_move(cell, resulting_left, 1, 0, 0, 0);
        } else if (isRightLastMoved && isLegalRight) {
            (*legal_moves)[4] = generate_move(cell, resulting_right, 1, 0, 0, 0);
        }
    }
}

void get_rook_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    bool is_attacker_white = is_piece_on_cell_white(cell);

    int file = cell % 8;
    int rank = cell / 8;
    int amount_of_moves = 0;

    // LOGIC -> loop through all 4 directions (top, bottom, left, right)
    // and keep adding to legal_moves until reaches obstacle
    // UP Loop
    for (int top = rank + 1; top < 8; top++) {
        int target_cell = file + top * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // DOWN loop
    for (int bottom = rank - 1; bottom >= 0; bottom--) {
        int target_cell = file + bottom * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // LEFT loop
    for (int left = file - 1; left >= 0; left--) {
        int target_cell = left + rank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // RIGHT loop
    for (int right = file + 1; right < 8; right++) {
        int target_cell = right + rank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }
}

void get_bishop_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    bool is_attacker_white = is_piece_on_cell_white(cell);
    int file = cell % 8;
    int rank = cell / 8;
    int amount_of_moves = 0;

    // LOGIC -> Almost exactly the same as the rook function (above this function),
    // However, instead of going in 4 directions (up, down, left, right), we do the
    // 4 diagonals.
    // ORDER -> right top, right bottom, left top, left bottom
    // RIGHT TOP LOOP
    for (int multi = 1; file + multi < 8 && rank + multi < 8; multi++) {
        int newFile = file + multi;
        int newRank = rank + multi;

        int target_cell = newFile + newRank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // RIGHT BOTTOM LOOP
    for (int multi = 1; file + multi < 8 && rank - multi >= 0; multi++) {
        int newFile = file + multi;
        int newRank = rank - multi;

        int target_cell = newFile + newRank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // LEFT TOP LOOP
    for (int multi = 1; file - multi >= 0 && rank + multi < 8; multi++) {
        int newFile = file - multi;
        int newRank = rank + multi;

        int target_cell = newFile + newRank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // LEFT BOTTOM LOOP
    for (int multi = 1; file - multi >= 0 && rank - multi >= 0; multi++) {
        int newFile = file - multi;
        int newRank = rank - multi;

        int target_cell = newFile + newRank * 8;

        bool is_white = is_piece_on_cell_white(target_cell);
        bool is_empty = is_cell_empty(target_cell);
        bool is_legal = is_valid_target(target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }
}

void get_knight_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    bool is_white = is_piece_on_cell_white(cell);
    uint8_t attacker_file = cell % 8;
    uint8_t attacker_rank = cell / 8;

    // It is hard to loop through all the legal moves of a knight with a simple
    // for loop, so instead, I am going to hardcode them, then check one by one whether they stand.
    std::unordered_map<uint8_t, uint8_t> potential_moves;
    if (attacker_rank < 6) {
        if (attacker_file < 7) potential_moves[cell + 17] = cell + 17;
        if (attacker_file > 0) potential_moves[cell + 15] = cell + 15;
    }

    if (attacker_file < 6) {
        if (attacker_rank < 7) potential_moves[cell + 10] = cell + 10;
        if (attacker_rank > 0) potential_moves[cell - 6] = cell - 6;
    }

    if (attacker_rank > 1) {
        if (attacker_file < 7) potential_moves[cell - 15] = cell - 15;
        if (attacker_file > 0) potential_moves[cell - 17] = cell - 17;
    }

    if (attacker_file > 1) {
        if (attacker_rank < 7) potential_moves[cell - 10] = cell - 10;
        if (attacker_rank > 0) potential_moves[cell + 6] = cell + 6;
    }

    int amount_of_moves = 0;
    for (auto pair : potential_moves) {
        uint8_t target_cell = pair.first;
        if (!is_valid_target(target_cell, is_white)) continue;

        int file = target_cell % 8;
        int rank = target_cell / 8;

        if (file == attacker_file || rank == attacker_rank) continue;

        bool is_capturing = !is_cell_empty(target_cell);
        (*legal_moves)[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, is_capturing);
        amount_of_moves++;
    }
}

void get_queen_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    // A very clever trick : A king's moves are just the combination of a rook's and a bishop's.
    // So that's what we're doing, just calling both the get_rook_legal_moves and the get_bishop_legal_moves
    // functions, then combining the result.

    uint16_t rook_legal_moves[27] = {};
    uint16_t bishop_legal_moves[27] = {};

    get_rook_legal_moves(&rook_legal_moves, cell);
    get_bishop_legal_moves(&bishop_legal_moves, cell);

    int last = -1;
    for (int i = 0; i < 27; i++) {
        if (rook_legal_moves[i] == 0) break;

        (*legal_moves)[i] = rook_legal_moves[i];
        last = i;
    }

    for (int i = last + 1; i < 27; i++) {
        if (bishop_legal_moves[i - last - 1] == 0) break;

        (*legal_moves)[i] = bishop_legal_moves[i - last - 1];
    }
}

void get_king_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    // We will make an offset array, then loop through all those offsets
    // to get the king's neighbors since a king's legal moves are just 1 cell in all the
    // directions around it.
    int neighbors[8] = {7, 8, 9, -1, 1, -9, -8, -7};
    bool is_attacker_white = is_piece_on_cell_white(cell);

    int amount_of_moves = 0;
    for (int i = 0; i < 8; i++) {
        int offset = neighbors[i];
        int neighbor = cell + offset;

        int file = neighbor % 8;
        int rank = neighbor / 8;

        int file_diff = file - (cell % 8);
        int rank_diff = rank - (cell / 8);

        // Make sure absolute difference is always less or equal to 1, since if a king
        // is beside the edge of the board, the moves can wrap around to the other side, which we don't want.
        bool is_file_cut = !(1 >= file_diff && file_diff >= -1);
        bool is_rank_cut = !(1 >= rank_diff && rank_diff >= -1);

        if (is_valid_target(neighbor, is_attacker_white) && !is_file_cut && !is_rank_cut) {
            bool is_capture = !is_cell_empty(neighbor);
            (*legal_moves)[amount_of_moves] = generate_move(cell, neighbor, 0, 0, 0, is_capture);
            amount_of_moves++;
        }
    }

    if (is_attacker_white) {
        if (is_white_long_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell - 1; check_cell > 0; check_cell--) {
                if (is_cell_empty(check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                (*legal_moves)[amount_of_moves] = generate_move(cell, cell - 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }

        if (is_white_short_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell + 1; check_cell < 7; check_cell++) {
                if (is_cell_empty(check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                (*legal_moves)[amount_of_moves] = generate_move(cell, cell + 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }
    } else {
        if (is_black_long_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell - 1; check_cell > 56; check_cell--) {
                if (is_cell_empty(check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                (*legal_moves)[amount_of_moves] = generate_move(cell, cell - 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }

        if (is_black_short_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell + 1; check_cell < 63; check_cell++) {
                if (is_cell_empty(check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                (*legal_moves)[amount_of_moves] = generate_move(cell, cell + 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }
    }
}

void get_piece_legal_moves(uint16_t (*legal_moves)[27], uint8_t cell) {
    std::memset(legal_moves, 0, sizeof(*legal_moves));
    if (cell < 0 || cell > 63) return;

    uint64_t* piece_type = get_piece_type_on_cell(cell);

    if (piece_type == &board.whitePawns || piece_type == &board.blackPawns) {
        get_pawn_legal_moves(legal_moves, cell);
    } else if (piece_type == &board.whiteRooks || piece_type == &board.blackRooks) {
        get_rook_legal_moves(legal_moves, cell);
    } else if (piece_type == &board.whiteKnights || piece_type == &board.blackKnights) {
        get_knight_legal_moves(legal_moves, cell);
    } else if (piece_type == &board.whiteBishops || piece_type == &board.blackBishops) {
        get_bishop_legal_moves(legal_moves, cell);
    } else if (piece_type == &board.whiteQueens || piece_type == &board.blackQueens) {
        get_queen_legal_moves(legal_moves, cell);
    } else if (piece_type == &board.whiteKing || piece_type == &board.blackKing) {
        get_king_legal_moves(legal_moves, cell);
    }

    std::cout << "Fetching succesful" << '\n';
}

void handle_move(uint64_t* piece_type, uint16_t move) {
    make_move(piece_type, move);

    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t destination_pos = (move >> 4) & 63ull;

    last_placed_cell = destination_pos;
    bool is_castle = move & (1ull << 2);

    is_en_passant_available = false;
    if (*piece_type == board.whitePawns && destination_pos == original_pos + 16)
        is_en_passant_available = true;
    else if (*piece_type == board.blackPawns && destination_pos == original_pos - 16)
        is_en_passant_available = true;

    if (*piece_type == board.whiteRooks) {
        if (original_pos == 0)
            is_white_long_castle_available = false;
        else if (original_pos == 7)
            is_white_short_castle_available = false;
    } else if (*piece_type == board.blackRooks) {
        if (original_pos == 56)
            is_black_long_castle_available = false;
        else if (original_pos == 63)
            is_black_short_castle_available = false;
    }

    if (*piece_type == board.whiteKing) {
        is_white_long_castle_available = false;
        is_white_short_castle_available = false;
    } else if (*piece_type == board.blackKing) {
        is_black_long_castle_available = false;
        is_black_short_castle_available = false;
    }

    uint8_t pawn_promotion_cell = get_promotion_pawn_cell();
    if (pawn_promotion_cell == 64) is_white_turn = !is_white_turn;
}

void make_move(uint64_t* piece_type, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_castle = (move & (1ull << 2));
    bool is_promotion = (move & (1ull << 1));
    bool is_capture = (move & 1);

    if (is_capture) {
        uint8_t attack_pos = (is_en_passant ? last_placed_cell : new_pos);
        remove_piece_on_cell(attack_pos);
    } else if (is_castle) {
        if (original_pos == new_pos + 2) {
            uint64_t* rook_type = get_piece_type_on_cell(new_pos - 2);

            *rook_type ^= (1ull << (new_pos - 2));
            *rook_type ^= (1ull << (new_pos + 1));
        } else if (original_pos == new_pos - 2) {
            uint64_t* rook_type = get_piece_type_on_cell(new_pos + 1);

            *rook_type ^= (1ull << (new_pos + 1));
            *rook_type ^= (1ull << (new_pos - 1));
        }
    }

    *piece_type ^= (1ull << original_pos);
    *piece_type ^= (1ull << new_pos);
}

void remove_piece_on_cell(uint8_t cell) {
    uint64_t* piece_type = get_piece_type_on_cell(cell);

    if (!piece_type) return;
    *piece_type ^= (1ull << cell);
}

void get_strictly_legal_moves(uint64_t* piece_type, uint16_t (*legal_moves)[27]) {
    uint16_t strictly_legal_moves[27] = {};

    uint8_t amount_of_moves = 0;
    for (uint16_t move : *legal_moves) {
        if (move == 0) break;

        bool is_white = is_piece_on_cell_white((move >> 10) & 63ull);
        bool is_check = (is_white && white_in_check) || (!is_white && black_in_check);

        bool is_castle = move & (1ull << 2);
        if (is_castle && is_check) continue;

        std::cout << "Check check" << '\n';
        bool does_move_cause_check = try_make_move_and_check_if_causes_check(piece_type, move);
        if (does_move_cause_check) continue;

        strictly_legal_moves[amount_of_moves] = move;
        amount_of_moves++;
        std::cout << "Checked move #" << (int)amount_of_moves << '\n';
    }

    for (int i = 0; i < 27; i++) {
        (*legal_moves)[i] = strictly_legal_moves[i];
    }

    std::cout << "Full Fetch" << '\n';
}

bool try_make_move_and_check_if_causes_check(uint64_t* piece_type, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_checking_white = is_piece_on_cell_white(original_pos);

    uint8_t attack_pos = (is_en_passant ? last_placed_cell : new_pos);
    uint64_t* attacked_piece_type = get_piece_type_on_cell(attack_pos);

    std::cout << "Finished fetching move headers\n";

    make_move(piece_type, move);
    std::cout << "Made move\n";
    bool is_check = is_in_check(is_checking_white);

    std::cout << "Made move and checked if check\n";

    *piece_type ^= (1ull << original_pos);
    *piece_type ^= (1ull << new_pos);

    std::cout << "Finished putting back pieces\n";

    if (attacked_piece_type) *attacked_piece_type ^= (1ull << attack_pos);

    return is_check;
}

bool is_in_check(bool is_checking_white) {
    uint8_t king_cell = 64;

    for (int cell = 0; cell < 64; cell++) {
        if ((is_checking_white ? board.whiteKing : board.blackKing) & (1ull << cell)) {
            king_cell = cell;
            std::cout << "Found\n";
            break;
        }
    }

    for (int cell = 0; cell < 64; cell++) {
        uint16_t legal_moves[27];
        get_piece_legal_moves(&legal_moves, cell);

        for (uint16_t move : legal_moves) {
            if (move == 0) break;
            std::cout << "found Move\n";
            uint8_t destination = (move >> 4) & 63ull;

            if (destination == king_cell) return true;
        }
    }

    return false;
}

uint16_t get_move_from_destination_in_legal_moves(uint16_t (*legal_moves)[27], uint8_t destination_to_check) {
    for (uint16_t move : *legal_moves) {
        if (move == 0) break;

        uint8_t destination = (move >> 4) & 63ull;
        if (destination == destination_to_check) return move;
    }

    return 0;
}
