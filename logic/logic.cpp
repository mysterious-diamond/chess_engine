#include "logic.h"

#include <cstdint>

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
