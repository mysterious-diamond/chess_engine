#include "logic.h"

#include <cstdint>
#include <unordered_map>

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
        get_piece_legal_moves(board, &legal_moves[0], 0, cell, 0);

        for (uint16_t move : legal_moves) {
            if (move == 0) break;
            uint8_t destination = (move >> 4) & 63ull;

            if (destination == king_cell) return true;
        }
    }

    return false;
}

void make_move(Board& board, uint16_t last_move, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_castle = (move & (1ull << 2));
    bool is_promotion = (move & (1ull << 1));
    bool is_capture = (move & 1);

    uint64_t* piece_type = get_piece_type_on_cell(board, original_pos);

    if (is_capture) {
        uint8_t attack_pos = (is_en_passant ? (last_move >> 4) & 64 : new_pos);
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

void remove_piece_on_cell(Board& board, uint8_t cell) {
    uint64_t* piece_type = get_piece_type_on_cell(board, cell);

    if (!piece_type) return;
    *piece_type ^= (1ull << cell);
}

void get_strictly_legal_moves(Board& board, uint16_t last_move, uint16_t* array_ptr, uint64_t* piece_type) {
    uint16_t strictly_legal_moves[27] = {};

    uint8_t amount_of_moves = 0;
    for (int i = 0; i < 27; i++) {
        uint16_t move = array_ptr[i];
        if (move == 0) break;

        bool is_white = is_piece_on_cell_white(board, (move >> 10) & 63ull);
        bool is_check = is_in_check(board, is_white);

        bool is_castle = move & (1ull << 2);
        if (is_castle && is_check) continue;

        bool does_move_cause_check = try_make_move_and_check_if_causes_check(board, piece_type, last_move, move);
        if (does_move_cause_check) continue;

        strictly_legal_moves[amount_of_moves] = move;
        amount_of_moves++;
    }

    for (int i = 0; i < 27; i++) {
        array_ptr[i] = strictly_legal_moves[i];
    }
}

bool try_make_move_and_check_if_causes_check(Board& board, uint64_t* piece_type, uint16_t last_move, uint16_t move) {
    uint8_t original_pos = (move >> 10) & 63ull;
    uint8_t new_pos = (move >> 4) & 63ull;

    bool is_en_passant = (move & (1ull << 3));
    bool is_castle = (move & (1ull << 2));
    bool is_checking_white = is_piece_on_cell_white(board, original_pos);

    uint8_t attack_pos = (is_en_passant ? (last_move >> 4) & 64 : new_pos);
    uint64_t* attacked_piece_type = get_piece_type_on_cell(board, attack_pos);

    make_move(board, last_move, move);
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

bool try_promote_pawn(Board& board, uint8_t chosen_promotion_type) {
    uint8_t pawn_promotion_cell = get_promotion_pawn_cell(board);
    if (pawn_promotion_cell == 64) return false;

    bool is_white = is_piece_on_cell_white(board, pawn_promotion_cell);
    uint64_t* piece_type = (is_white ? &board.white_pawns : &board.black_pawns);

    *piece_type ^= (1ull << pawn_promotion_cell);
    if (chosen_promotion_type == 1) {
        uint64_t* queen_type = (is_white ? &board.white_queens : &board.black_queens);
        *queen_type ^= (1ull << pawn_promotion_cell);
    } else if (chosen_promotion_type == 2) {
        uint64_t* rook_type = (is_white ? &board.white_rooks : &board.black_rooks);
        *rook_type ^= (1ull << pawn_promotion_cell);
    } else if (chosen_promotion_type == 3) {
        uint64_t* knight_type = (is_white ? &board.white_bishops : &board.black_knights);
        *knight_type ^= (1ull << pawn_promotion_cell);
    } else if (chosen_promotion_type == 4) {
        uint64_t* bishop_type = (is_white ? &board.white_bishops : &board.black_bishops);
        *bishop_type ^= (1ull << pawn_promotion_cell);
    } else {
        return false;
    }

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
