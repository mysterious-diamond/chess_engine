#pragma once

#include <cstdint>

extern "C" {
uint16_t get_move_from_destination_in_legal_moves(uint16_t* array_ptr, uint8_t destination_to_check);
uint16_t generate_move(uint8_t original_pos, uint8_t destination, bool is_en_passant, bool is_castle, bool is_promotion,
                       bool is_capture);
}
