cmake_minimum_required(VERSION 3.12)
project(Chip8Emulator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -O2)

find_package(SDL2 REQUIRED)

set(SOURCES
    main.c
    cpu.c
    config.c
    SDL/video.c
    SDL/audio.c
    SDL/keypad.c
)

add_executable(chip8 ${SOURCES})

target_include_directories(chip8 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/SDL
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(chip8 PRIVATE SDL2::SDL2 m)
