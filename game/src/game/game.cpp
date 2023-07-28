#include "game/game.hpp"

#include <kat/window/window.hpp>
#include <X11/extensions/Xrandr.h>
#include <X11/Xresource.h>


#include <spdlog/cfg/env.h>

#include <iostream>
#include <cstring>

int main() {
    spdlog::cfg::load_env_levels();

    std::shared_ptr<kat::window::windowing_engine> windowing_engine = kat::window::windowing_engine::create();

    auto* display = windowing_engine->platform->display;
    auto screen = windowing_engine->platform->screen_id;
    auto root_window = windowing_engine->platform->root;

    auto monitors = windowing_engine->monitors();

    auto* screen_resources = XRRGetScreenResources(display, root_window);



    std::cout << "Found " << monitors.size() << " monitors:\n";
    int i_ = 0;
    for (const auto& monitor : monitors) {
        auto size = monitor->size();
        auto psize = monitor->physical_size();
        auto pos = monitor->position();

        std::cout << "  Monitor #" << i_ << ": " << monitor->name() << "\n";
        std::cout << "    Size: " << size.x << " x " << size.y << '\n';
        std::cout << "    Virtual Location: " << pos.x << ", " << pos.y << '\n';
        std::cout << "    Physical Size: " << psize.x << " x " << psize.y << '\n';

        if (monitor->is_primary()) {
            std::cout << "    Primary\n";
        }

        i_++;
    }

    auto dpi = windowing_engine->platform->dpi();
    auto scale = windowing_engine->platform->scale();

    std::cout << "System DPI: " << dpi.x << " x " << dpi.y << '\n';
    std::cout << "Scale: " << scale.x << " x " << scale.y << '\n';

    return EXIT_SUCCESS;
}