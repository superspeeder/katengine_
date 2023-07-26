cmake_minimum_required(VERSION 3.24)
project(katengine_sample_game VERSION 0.0.1)

add_executable(sample_game src/game/game.cpp src/game/game.hpp)
target_include_directories(sample_game PRIVATE src/)

target_link_libraries(sample_game katengine::katengine X11 GL Xrandr)
