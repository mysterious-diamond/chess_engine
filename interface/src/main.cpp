// #include <cstdint>
// #include <iostream>

#include "game.h"
#include "raylib.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess Engine");
    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();

        game.draw_game();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
