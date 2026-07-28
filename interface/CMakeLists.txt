cmake_minimum_required(VERSION 3.10)
project(chess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RUST_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../engine")
set(RUST_ENGINE_LIB "${RUST_ENGINE_DIR}/target/x86_64-pc-windows-gnu/release/libchess_engine.a")
add_executable(chess
    src/main.cpp
    src/game.cpp
    ../logic/logic.cpp
)

target_sources(chess PRIVATE
    src/game.h
    ../logic/logic.h
)

target_include_directories(chess PRIVATE
    raylib/include
    "${RUST_ENGINE_DIR}"
)

target_link_libraries(chess PRIVATE
    ${RUST_ENGINE_LIB}
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib/include/libraylib.a
    opengl32
    gdi32
    winmm
    ws2_32
    userenv
    bcrypt
    advapi32
    ntdll
)

add_custom_target(rust_engine ALL
    COMMAND cargo build --release --target x86_64-pc-windows-gnu
    WORKING_DIRECTORY ${RUST_ENGINE_DIR}
    COMMENT "Building Rust engine for MinGW..."
)

add_dependencies(chess rust_engine)
