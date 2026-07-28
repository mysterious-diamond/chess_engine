// #include <cstdint>
// #include <iostream>

#include "game.h"
#include "raylib.h"

int main() {
    SetTargetFPS(144);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Engine");
    Game game = Game(true);

    while (!WindowShouldClose()) {
        BeginDrawing();

        game.step_game();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
