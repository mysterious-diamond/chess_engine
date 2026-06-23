cmake_minimum_required(VERSION 3.10)
project(chess-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(chess-engine
    src/main.cpp
)

target_sources(chess-engine PRIVATE
    src/game.h
    src/game.cpp
)

target_include_directories(chess-engine PRIVATE
    C:/raylib/raylib/src
)

target_link_libraries(chess-engine PRIVATE
    C:/raylib/raylib/src/libraylib.a
    opengl32
    gdi32
    winmm
)
