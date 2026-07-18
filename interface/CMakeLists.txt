cmake_minimum_required(VERSION 3.10)
project(chess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(chess
    src/main.cpp
)

target_sources(chess PRIVATE
    src/game.h
    src/game.cpp
    ../logic/logic.h
    ../logic/logic.cpp
)

target_include_directories(chess PRIVATE
    C:/raylib/raylib/src
)

target_link_libraries(chess PRIVATE
    C:/raylib/raylib/src/libraylib.a
    opengl32
    gdi32
    winmm
)
