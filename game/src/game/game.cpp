#include "game/game.hpp"
#include "spdlog/spdlog.h"

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


    auto window = std::make_shared<kat::window::window>(windowing_engine, "hello!", glm::uvec2{800, 800}, glm::ivec2(100, 100));

    while (!windowing_engine->is_app_exit()) {
        windowing_engine->process_events();
    }


//    XEvent event;
//
//    auto escape_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_Escape);
//    auto space_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_space);
//    auto s_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_s);
//    auto w_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_w);
//    auto a_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_a);
//    auto d_kc = XKeysymToKeycode(windowing_engine->platform->display, XK_d);
//
//
//    while (true) {
//        XNextEvent(windowing_engine->platform->display, &event);
//        if (event.type == KeyPress) {
//            auto keycode = event.xkey.keycode;
//            if (keycode == escape_kc) {
//                std::cout << "Closing!" << std::endl;
//                break;
//            } else if (keycode == space_kc) {
//                auto size = window->size();
//                auto pos = window->position();
//                SPDLOG_INFO("{} x {} @ {}, {}", size.x, size.y, pos.x, pos.y);
//            } else if (keycode == s_kc) {
//                window->size({800, 600});
//            } else if (keycode == w_kc) {
//                window->size({600, 800});
//            } else if (keycode == a_kc) {
//                window->position(window->position() + glm::ivec2(-100, 0));
//            } else if (keycode == d_kc) {
//                window->position(window->position() + glm::ivec2(100, 0));
//            }
//        }
//    }

    return EXIT_SUCCESS;
}