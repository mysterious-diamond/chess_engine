#include "logic.h"

#include <cstdint>
#include <unordered_map>

uint8_t last_placed_cell = 64;

bool is_en_passant_available = false;
bool is_white_turn = true;

bool white_in_check = false;
bool black_in_check = false;

bool is_white_long_castle_available = true;
bool is_white_short_castle_available = true;

bool is_black_long_castle_available = true;
bool is_black_short_castle_available = true;

bool is_in_check(Board board, bool is_checking_white) {
    uint8_t king_cell = 64;

    for (int cell = 0; cell < 64; cell++) {
        if ((is_checking_white ? board.white_king : board.black_king) & (1ull << cell)) {
            king_cell = cell;
            break;
        }
    }

    for (int cell = 0; cell < 64; cell++) {
        uint16_t legal_moves[27] = {};
        get_piece_legal_moves(board, &legal_moves[0], cell);

        for (uint16_t move : legal_moves) {
            if (move == 0) break;
            uint8_t destination = (move >> 4) & 63ull;

            if (destination == king_cell) return true;
        }
    }

    return false;
}

void get_piece_legal_moves(Board& board, uint16_t* array_ptr, uint8_t cell) {
    uint16_t legal_moves[27] = {};
    if (cell < 0 || cell > 63) return;

    uint64_t* piece_type = get_piece_type_on_cell(board, cell);

    if (piece_type == &board.white_pawns || piece_type == &board.black_pawns) {
        get_pawn_legal_moves(board, legal_moves, cell);
    } else if (piece_type == &board.white_rooks || piece_type == &board.black_rooks) {
        get_rook_legal_moves(board, legal_moves, cell);
    } else if (piece_type == &board.white_knights || piece_type == &board.black_knights) {
        get_knight_legal_moves(board, legal_moves, cell);
    } else if (piece_type == &board.white_bishops || piece_type == &board.black_bishops) {
        get_bishop_legal_moves(board, legal_moves, cell);
    } else if (piece_type == &board.white_queens || piece_type == &board.black_queens) {
        get_queen_legal_moves(board, legal_moves, cell);
    } else if (piece_type == &board.white_king || piece_type == &board.black_king) {
        get_king_legal_moves(board, legal_moves, cell);
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_pawn_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    // Make sure array is set to 0 so it doesnt return garbage values
    uint16_t legal_moves[27] = {};
    bool is_white = is_piece_on_cell_white(board, cell);
    int direction = (is_white ? 1 : -1);
    int rank = cell / 8;
    int moveN = 0;

    int target_cell = cell + 8 * direction;
    if (is_cell_empty(board, target_cell) && is_valid_target(board, target_cell, is_white)) {
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));

        legal_moves[moveN] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 0);
        moveN++;

        target_cell = cell + 16 * direction;
        bool is_pawn_at_start = (is_white ? rank == 1 : rank == 6);

        // if pawn has never moved before, it can move 2 steps forward
        if (is_pawn_at_start && is_cell_empty(board, target_cell) && is_valid_target(board, target_cell, is_white)) {
            legal_moves[moveN] = generate_move(cell, target_cell, 0, 0, 0, 0);
            moveN++;
        }
    }

    // Left top square checking
    target_cell = cell + 7 * direction;
    bool is_empty = is_cell_empty(board, target_cell);
    if (is_valid_target(board, target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));
        legal_moves[moveN] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 1);
        moveN++;
    }

    // Check Right Top Square For Enemy
    target_cell = cell + 9 * direction;
    is_empty = is_cell_empty(board, target_cell);
    if (is_valid_target(board, target_cell, is_white) && !is_empty && rank != target_cell / 8) {
        bool is_cell_on_last_rank = (target_cell / 8 == (is_white ? 7 : 0));
        legal_moves[moveN] = generate_move(cell, target_cell, 0, 0, is_cell_on_last_rank, 1);
        moveN++;
    }

    if (is_en_passant_available) try_insert_pawn_enpassant_move(board, legal_moves, cell, moveN);

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_rook_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    uint16_t legal_moves[27] = {};
    bool is_attacker_white = is_piece_on_cell_white(board, cell);

    int file = cell % 8;
    int rank = cell / 8;
    int amount_of_moves = 0;

    // LOGIC -> loop through all 4 directions (top, bottom, left, right)
    // and keep adding to legal_moves until reaches obstacle
    // UP Loop
    for (int top = rank + 1; top < 8; top++) {
        int target_cell = file + top * 8;

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // DOWN loop
    for (int bottom = rank - 1; bottom >= 0; bottom--) {
        int target_cell = file + bottom * 8;

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // LEFT loop
    for (int left = file - 1; left >= 0; left--) {
        int target_cell = left + rank * 8;

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    // RIGHT loop
    for (int right = file + 1; right < 8; right++) {
        int target_cell = right + rank * 8;

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_bishop_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    uint16_t legal_moves[27] = {};

    bool is_attacker_white = is_piece_on_cell_white(board, cell);
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

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
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

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
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

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
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

        bool is_white = is_piece_on_cell_white(board, target_cell);
        bool is_empty = is_cell_empty(board, target_cell);
        bool is_legal = is_valid_target(board, target_cell, is_attacker_white);

        if (is_empty && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 0);
            amount_of_moves++;
        } else if (is_white != is_attacker_white && is_legal) {
            legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, 1);
            amount_of_moves++;
            break;
        } else {
            break;
        }
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_knight_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    uint16_t legal_moves[27] = {};

    bool is_white = is_piece_on_cell_white(board, cell);
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
        if (!is_valid_target(board, target_cell, is_white)) continue;

        int file = target_cell % 8;
        int rank = target_cell / 8;

        if (file == attacker_file || rank == attacker_rank) continue;

        bool is_capturing = !is_cell_empty(board, target_cell);
        legal_moves[amount_of_moves] = generate_move(cell, target_cell, 0, 0, 0, is_capturing);
        amount_of_moves++;
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_queen_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    // A very clever trick : A king's moves are just the combination of a rook's and a bishop's.
    // So that's what we're doing, just calling both the get_rook_legal_moves and the get_bishop_legal_moves
    // functions, then combining the result.

    uint16_t legal_moves[27] = {};
    uint16_t rook_legal_moves[27] = {};
    uint16_t bishop_legal_moves[27] = {};

    get_rook_legal_moves(board, &rook_legal_moves[0], cell);
    get_bishop_legal_moves(board, &bishop_legal_moves[0], cell);

    int last = -1;
    for (int i = 0; i < 27; i++) {
        if (rook_legal_moves[i] == 0) break;

        legal_moves[i] = rook_legal_moves[i];
        last = i;
    }

    for (int i = last + 1; i < 27; i++) {
        if (bishop_legal_moves[i - last - 1] == 0) break;

        legal_moves[i] = bishop_legal_moves[i - last - 1];
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void get_king_legal_moves(Board board, uint16_t* array_ptr, uint8_t cell) {
    // We will make an offset array, then loop through all those offsets
    // to get the king's neighbors since a king's legal moves are just 1 cell in all the
    // directions around it.
    uint16_t legal_moves[27] = {};

    int neighbors[8] = {7, 8, 9, -1, 1, -9, -8, -7};
    bool is_attacker_white = is_piece_on_cell_white(board, cell);

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

        if (is_valid_target(board, neighbor, is_attacker_white) && !is_file_cut && !is_rank_cut) {
            bool is_capture = !is_cell_empty(board, neighbor);
            legal_moves[amount_of_moves] = generate_move(cell, neighbor, 0, 0, 0, is_capture);
            amount_of_moves++;
        }
    }

    if (is_attacker_white) {
        if (is_white_long_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell - 1; check_cell > 0; check_cell--) {
                if (is_cell_empty(board, check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                legal_moves[amount_of_moves] = generate_move(cell, cell - 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }

        if (is_white_short_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell + 1; check_cell < 7; check_cell++) {
                if (is_cell_empty(board, check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                legal_moves[amount_of_moves] = generate_move(cell, cell + 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }
    } else {
        if (is_black_long_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell - 1; check_cell > 56; check_cell--) {
                if (is_cell_empty(board, check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                legal_moves[amount_of_moves] = generate_move(cell, cell - 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }

        if (is_black_short_castle_available) {
            bool blockedPath = false;
            for (int check_cell = cell + 1; check_cell < 63; check_cell++) {
                if (is_cell_empty(board, check_cell)) continue;
                blockedPath = true;
                break;
            }

            if (!blockedPath) {
                legal_moves[amount_of_moves] = generate_move(cell, cell + 2, 0, 1, 0, 0);
                amount_of_moves++;
            }
        }
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = legal_moves[i];
    }
}

void handle_move(Board& board, uint64_t* piece_type, uint16_t move) {
    make_move(board, move);

    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t destination_pos = (move >> 4) & 63ull;

    last_placed_cell = destination_pos;
    bool is_castle = move & (1ull << 2);

    is_en_passant_available = false;
    if (*piece_type == board.white_pawns && destination_pos == original_pos + 16)
        is_en_passant_available = true;
    else if (*piece_type == board.black_pawns && destination_pos == original_pos - 16)
        is_en_passant_available = true;

    if (*piece_type == board.white_rooks) {
        if (original_pos == 0)
            is_white_long_castle_available = false;
        else if (original_pos == 7)
            is_white_short_castle_available = false;
    } else if (*piece_type == board.black_rooks) {
        if (original_pos == 56)
            is_black_long_castle_available = false;
        else if (original_pos == 63)
            is_black_short_castle_available = false;
    }

    if (*piece_type == board.white_king) {
        is_white_long_castle_available = false;
        is_white_short_castle_available = false;
    } else if (*piece_type == board.black_king) {
        is_black_long_castle_available = false;
        is_black_short_castle_available = false;
    }

    uint8_t pawn_promotion_cell = get_promotion_pawn_cell(board);
    if (pawn_promotion_cell == 64) is_white_turn = !is_white_turn;
}

void try_insert_pawn_enpassant_move(Board board, uint16_t (&legal_moves)[27], uint8_t cell, int moveN) {
    // If enemy moved a pawn 2 spaces forward to beside a friendly pawn,
    // the friendly pawn can move to the space the pawn skipped and
    // capture the pawn.
    bool is_white = is_piece_on_cell_white(board, cell);
    int rank = cell / 8;

    int left = (is_white ? cell - 1 : cell + 1);
    int right = (is_white ? cell + 1 : cell - 1);

    int leftRank = left / 8;
    int rightRank = right / 8;

    int resulting_left = (is_white ? cell + 7 : cell - 7);
    int resulting_right = (is_white ? cell + 9 : cell - 9);

    bool isLeftLastMoved = (left == last_placed_cell);
    bool isRightLastMoved = (right == last_placed_cell);

    bool isLegalLeft = is_valid_target(board, left, is_white) && rank == leftRank;
    bool isLegalRight = is_valid_target(board, right, is_white) && rank == rightRank;

    if (isLeftLastMoved && isLegalLeft) {
        legal_moves[moveN] = generate_move(cell, resulting_left, 1, 0, 0, 1);
    } else if (isRightLastMoved && isLegalRight) {
        legal_moves[moveN] = generate_move(cell, resulting_right, 1, 0, 0, 1);
    }
};

void make_move(Board& board, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_castle = (move & (1ull << 2));
    bool is_promotion = (move & (1ull << 1));
    bool is_capture = (move & 1);

    uint64_t* piece_type = get_piece_type_on_cell(board, original_pos);

    if (is_capture) {
        uint8_t attack_pos = (is_en_passant ? last_placed_cell : new_pos);
        remove_piece_on_cell(board, attack_pos);
    } else if (is_castle) {
        if (original_pos == new_pos + 2) {
            uint64_t* rook_type = get_piece_type_on_cell(board, new_pos - 2);

            *rook_type ^= (1ull << (new_pos - 2));
            *rook_type ^= (1ull << (new_pos + 1));
        } else if (original_pos == new_pos - 2) {
            uint64_t* rook_type = get_piece_type_on_cell(board, new_pos + 1);

            *rook_type ^= (1ull << (new_pos + 1));
            *rook_type ^= (1ull << (new_pos - 1));
        }
    }

    *piece_type ^= (1ull << original_pos);
    *piece_type ^= (1ull << new_pos);
}

double evaluate_board(Board& board, bool is_evaluating_white) {
    double score = 0.0;
    for (int i = 0; i < 64; i++) {
        uint64_t* piece_type = get_piece_type_on_cell(board, i);
        if (!piece_type) continue;

        int material_score = get_material_score_of_piece(board, i);
        int PST_score = get_PST_score_of_piece(board, i);

        score += material_score + PST_score;
    }

    return (is_evaluating_white ? score : -score);
}

void remove_piece_on_cell(Board& board, uint8_t cell) {
    uint64_t* piece_type = get_piece_type_on_cell(board, cell);

    if (!piece_type) return;
    *piece_type ^= (1ull << cell);
}

void get_strictly_legal_moves(Board& board, uint16_t* array_ptr, uint64_t* piece_type) {
    uint16_t strictly_legal_moves[27] = {};

    uint8_t amount_of_moves = 0;
    for (int i = 0; i < 27; i++) {
        uint16_t move = array_ptr[i];
        if (move == 0) break;

        bool is_white = is_piece_on_cell_white(board, (move >> 10) & 63ull);
        bool is_check = (is_white && white_in_check) || (!is_white && black_in_check);

        bool is_castle = move & (1ull << 2);
        if (is_castle && is_check) continue;

        bool does_move_cause_check = try_make_move_and_check_if_causes_check(board, piece_type, move);
        if (does_move_cause_check) continue;

        strictly_legal_moves[amount_of_moves] = move;
        amount_of_moves++;
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = strictly_legal_moves[i];
    }
}

bool try_make_move_and_check_if_causes_check(Board& board, uint64_t* piece_type, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_castle = (move & (1ull << 2));
    bool is_checking_white = is_piece_on_cell_white(board, original_pos);

    uint8_t attack_pos = (is_en_passant ? last_placed_cell : new_pos);
    uint64_t* attacked_piece_type = get_piece_type_on_cell(board, attack_pos);

    make_move(board, move);
    bool is_check = is_in_check(board, is_checking_white);

    *piece_type ^= (1ull << original_pos);
    *piece_type ^= (1ull << new_pos);

    if (is_castle) {
        if (original_pos == new_pos + 2) {
            uint64_t* rook_type = get_piece_type_on_cell(board, new_pos + 1);

            *rook_type ^= (1ull << (new_pos + 1));
            *rook_type ^= (1ull << (new_pos - 2));
        } else if (original_pos == new_pos - 2) {
            uint64_t* rook_type = get_piece_type_on_cell(board, new_pos - 1);

            *rook_type ^= (1ull << (new_pos - 1));
            *rook_type ^= (1ull << (new_pos + 1));
        }
    } else if (attacked_piece_type) {
        *attacked_piece_type ^= (1ull << attack_pos);
    }

    return is_check;
}

void handle_promotion(Board board, uint64_t* chosen_promotion_type) {
    if (try_promote_pawn(board, chosen_promotion_type)) is_white_turn = !is_white_turn;
}

bool try_promote_pawn(Board board, uint64_t* chosen_promotion_type) {
    uint8_t pawn_promotion_cell = get_promotion_pawn_cell(board);
    if (pawn_promotion_cell == 64) return false;

    bool is_white = is_piece_on_cell_white(board, pawn_promotion_cell);
    uint64_t* piece_type = (is_white ? &board.white_pawns : &board.black_pawns);

    *piece_type ^= (1ull << pawn_promotion_cell);
    *chosen_promotion_type ^= (1ull << pawn_promotion_cell);
    return true;
}

double get_PST_score_of_piece(Board board, uint8_t cell) {
    const int pawnPST[64] = {0,  0,   0,  0, 0,  0,  0,  0,   50,  50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30,  20,
                             10, 10,  5,  5, 10, 25, 25, 10,  5,   5,  0,  0,  0,  20, 20, 0,  0,  0,  5,  -5, -10, 0,
                             0,  -10, -5, 5, 5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,  0,  0,  0,  0,  0};
    const int rookPST[64] = {0, 0,  0,  0,  0,  0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0,  0,  0, 0, 0,
                             0, -5, -5, 0,  0,  0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0, 0,  -5, -5, 0, 0, 0,
                             0, 0,  0,  -5, -5, 0, 0, 0, 0, 0,  0,  -5, 0,  0,  0,  5, 5,  0,  0,  0};
    const int knightPST[64] = {-50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,   0,   -20, -40,
                               -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,   15,  20,  20,  15,  5,   -30,
                               -30, 0,   15,  20,  20,  15,  0,   -30, -30, 5,   10,  15,  15,  10,  5,   -30,
                               -40, -20, 0,   5,   5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};
    const int bishopPST[64] = {-20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,   0,   0,   -10,
                               -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,   5,   10,  10,  5,   5,   -10,
                               -10, 0,   10,  10,  10,  10,  0,   -10, -10, 10,  10,  10,  10,  10,  10,  -10,
                               -10, 5,   0,   0,   0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};
    const int queenPST[64] = {-20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,  0, 0,   0,   0,   0,   -10, -10, 0,   5,   5,  5, 5,
                              0,   -10, -5,  0,   5,   5,   5,   5,   0,   -5, 0, 0,   5,   5,   5,   5,   0,   -5,  -10, 5,  5, 5,
                              5,   5,   0,   -10, -10, 0,   5,   0,   0,   0,  0, -10, -20, -10, -10, -5,  -5,  -10, -10, -20};
    const int kingEarlyMidGamePST[64] = {-30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
                                         -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
                                         -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20, -20, -20, -20, -10,
                                         20,  20,  0,   0,   0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};
    const int kingEndGamePST[64] = {-50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,   -10, -20, -30,
                                    -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30,
                                    -30, -10, 30,  40,  40,  30,  -10, -30, -30, -10, 20,  30,  30,  20,  -10, -30,
                                    -30, -30, 0,   0,   0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50};

    bool is_white = is_piece_on_cell_white(board, cell);
    int pst_index = (is_white ? cell : 63 - cell);
    int game_phase = get_game_phase(board);
    uint64_t piece_type = *get_piece_type_on_cell(board, cell);
    double score;

    if (piece_type == board.white_pawns || piece_type == board.black_pawns) score = pawnPST[pst_index];
    if (piece_type == board.white_rooks || piece_type == board.black_rooks) score = rookPST[pst_index];
    if (piece_type == board.white_knights || piece_type == board.black_knights) score = knightPST[pst_index];
    if (piece_type == board.white_bishops || piece_type == board.black_bishops) score = bishopPST[pst_index];
    if (piece_type == board.white_queens || piece_type == board.black_queens) score = queenPST[pst_index];
    if (piece_type == board.white_king || piece_type == board.black_king) {
        score = (double)kingEarlyMidGamePST[pst_index] * (256 - game_phase) / 256;
        score += (double)kingEndGamePST[pst_index] * game_phase / 256;
    }

    return (is_piece_on_cell_white(board, cell) ? score : -score);
}

bool is_valid_target(Board board, uint8_t cell, bool is_attacker_white) {
    if (cell < 0 || cell >= 64) return false;
    if (!is_cell_empty(board, cell) && is_piece_on_cell_white(board, cell) == is_attacker_white) return false;
    return true;
}

bool is_cell_empty(Board& board, uint8_t cell) { return !get_piece_type_on_cell(board, cell); }

bool is_piece_on_cell_white(Board& board, uint8_t cell) {
    if (!get_piece_type_on_cell(board, cell)) return false;
    uint64_t piece_type = *get_piece_type_on_cell(board, cell);

    if (piece_type == board.white_pawns) return true;
    if (piece_type == board.white_rooks) return true;
    if (piece_type == board.white_knights) return true;
    if (piece_type == board.white_bishops) return true;
    if (piece_type == board.white_queens) return true;
    if (piece_type == board.white_king) return true;

    return false;
}

uint8_t get_promotion_pawn_cell(Board board) {
    for (int i = 56; i <= 63; i++) {
        if (board.white_pawns & (1ull << i)) {
            return i;
        }
    }

    for (int i = 0; i <= 7; i++) {
        if (board.black_pawns & (1ull << i)) {
            return i;
        }
    }

    return 64;
}

uint16_t get_move_from_destination_in_legal_moves(uint16_t* array_ptr, uint8_t destination_to_check) {
    for (int i = 0; i < 27; i++) {
        uint16_t move = array_ptr[i];
        if (move == 0) break;

        uint8_t destination = (move >> 4) & 63ull;
        if (destination == destination_to_check) return move;
    }

    return 0;
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

uint64_t* get_piece_type_on_cell(Board& board, uint8_t cell) {
    if (cell >= 64) return nullptr;

    if (board.white_pawns & (1ull << cell)) return &board.white_pawns;
    if (board.white_rooks & (1ull << cell)) return &board.white_rooks;
    if (board.white_knights & (1ull << cell)) return &board.white_knights;
    if (board.white_bishops & (1ull << cell)) return &board.white_bishops;
    if (board.white_queens & (1ull << cell)) return &board.white_queens;
    if (board.white_king & (1ull << cell)) return &board.white_king;

    if (board.black_pawns & (1ull << cell)) return &board.black_pawns;
    if (board.black_rooks & (1ull << cell)) return &board.black_rooks;
    if (board.black_knights & (1ull << cell)) return &board.black_knights;
    if (board.black_bishops & (1ull << cell)) return &board.black_bishops;
    if (board.black_queens & (1ull << cell)) return &board.black_queens;
    if (board.black_king & (1ull << cell)) return &board.black_king;

    return nullptr;
}

double get_material_score_of_piece(Board board, uint8_t cell) {
    const int pawnMaterialScore = 100;
    const int rookMaterialScore = 500;
    const int knightMaterialScore = 320;
    const int bishopMaterialScore = 330;
    const int queenMaterialScore = 900;

    uint64_t piece_type = *get_piece_type_on_cell(board, cell);
    double score = 0.0;

    if (piece_type == board.white_pawns || piece_type == board.black_pawns) score += pawnMaterialScore;
    if (piece_type == board.white_rooks || piece_type == board.black_rooks) score += rookMaterialScore;
    if (piece_type == board.white_knights || piece_type == board.black_knights) score += knightMaterialScore;
    if (piece_type == board.white_bishops || piece_type == board.black_bishops) score += bishopMaterialScore;
    if (piece_type == board.white_queens || piece_type == board.black_queens) score += queenMaterialScore;

    return (is_piece_on_cell_white(board, cell) ? score : -score);
}

int get_game_phase(Board board) {
    int pawns = 0, rooks = 0, knights = 0, bishops = 0, queens = 0;
    for (int i = 0; i < 64; i++) {
        if (board.white_pawns & (1ull << i) || board.black_pawns & (1ull << i)) pawns++;
        if (board.white_rooks & (1ull << i) || board.black_rooks & (1ull << i)) rooks++;
        if (board.white_knights & (1ull << i) || board.black_knights & (1ull << i)) knights++;
        if (board.white_bishops & (1ull << i) || board.black_bishops & (1ull << i)) bishops++;
        if (board.white_queens & (1ull << i) || board.black_queens & (1ull << i)) queens++;
    }

    int gamePhase = pawns + knights + bishops + rooks * 2 + queens * 4;
    gamePhase = 24 - gamePhase;
    if (gamePhase > 24) gamePhase = 24;
    if (gamePhase < 0) gamePhase = 0;

    // Scale from 0-24 range to 0-256 range proportionally
    gamePhase = gamePhase * (256 / 24);
    return gamePhase;
}
