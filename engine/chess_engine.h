// engine/chess_engine.h
#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "../logic/logic.h"
#include <cstdint>

extern "C" {
    uint16_t get_engine_move(Board board, uint8_t search_depth, uint16_t last_move, bool is_team_white);
}

#endif
