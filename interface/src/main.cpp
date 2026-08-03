// #include <cstdint>
// #include <iostream>

#include <iostream>

#include "game.h"
#include "raylib.h"

int main() {
    bool is_engine_move = false;
    std::string ans;

    std::cout << "Engine mode? (y/n)";
    std::cin >> ans;

    if (ans == "y") is_engine_move = true;

    SetTraceLogLevel(LOG_NONE);
    SetTargetFPS(144);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Engine");

    Game game = Game(is_engine_move);

    while (!WindowShouldClose()) {
        BeginDrawing();

        game.step_game();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
