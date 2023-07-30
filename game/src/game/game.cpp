#include "game/game.hpp"

#include <kat/window/window.hpp>

#include <spdlog/cfg/env.h>

#include <iostream>
#include <cstring>

int main() {
    spdlog::cfg::load_env_levels();

    std::shared_ptr<kat::window::windowing_engine> windowing_engine = kat::window::windowing_engine::create();

    auto monitors = windowing_engine->monitors();

    printf("Found %zu monitors\n", monitors.size());
    int i = 0;
    for (const auto& mon : monitors) {
        printf("Monitor #%d: %s\n", i, mon->name().data());
        printf("  Size: %d x %d\n", mon->size().x, mon->size().y);
        printf("  Physical Size: %d x %d\n", mon->physical_size().x, mon->physical_size().y);
        printf("  Position: %d x %d\n", mon->position().x, mon->position().y);
        printf("  Dpi: %f x %f\n", mon->dpi().x, mon->dpi().y);
        printf("  Scale: %f x %f\n", mon->scale().x, mon->scale().y);
        printf("  Primary: %s\n", mon->is_primary() ? "true" : "false");
        printf("  Video Modes:\n");
        kat::window::video_mode current = mon->video_mode();
        auto video_modes = mon->video_modes();
        for (const auto& vm : video_modes) {
            if (vm == current) {
                printf("    %d x %d @ %d [%d,%d,%d]*\n", vm.resolution.x, vm.resolution.y, vm.refresh_rate, vm.depth.red, vm.depth.green, vm.depth.blue);
            } else {
                printf("    %d x %d @ %d [%d,%d,%d]\n", vm.resolution.x, vm.resolution.y, vm.refresh_rate, vm.depth.red, vm.depth.green, vm.depth.blue);
            }
        }
    }


    return EXIT_SUCCESS;
}