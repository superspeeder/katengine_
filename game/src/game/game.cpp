#include "game/game.hpp"

#include <kat/window/window.hpp>
#include <spdlog/cfg/env.h>

#include <iostream>

int main() {
    spdlog::cfg::load_env_levels();

    return EXIT_SUCCESS;
}