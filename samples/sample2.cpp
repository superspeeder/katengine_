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
        auto pos = monitor->physical_size();

        std::cout << "  Monitor #" << i_ << ":\n";
        std::cout << "    Size: " << size.x << " x " << size.y << '\n';
        std::cout << "    Virtual Location: " << pos.x << ", " << pos.y << '\n';
        std::cout << "    Physical Size: " << psize.x << " x " << psize.y << '\n';
        std::cout << "    " << monitor->outputs().size() << " Outputs:\n";
        for (RROutput output : monitor->outputs()) {
            auto* output_info = XRRGetOutputInfo(display, screen_resources, output);
            std::cout << "      Name: " << output_info->name << '\n';
        }

        if (monitor->is_primary()) {
            std::cout << "    Primary\n";
        }

        i_++;
    }

    std::unordered_map<RRMode, XRRModeInfo> mode_infos;
    for (int i = 0 ; i < screen_resources->nmode ; i++) {
        auto modeinfo = screen_resources->modes[i];
        mode_infos[modeinfo.id] = modeinfo;
    }

    std::cout << "Screen Outputs: " << screen_resources->noutput << '\n';
    for (int i = 0 ;i < screen_resources->noutput ; i++) {
        auto output = screen_resources->outputs[i];
        auto output_info = XRRGetOutputInfo(display, screen_resources, output);
        std::cout << "Output #" << i << " (" << output << ")\n";
        std::cout << "  Name: " << output_info->name << '\n';
        std::cout << "  Clones: ";
        for (int j = 0; j < output_info->nclone ; j++) {
            std::cout << output_info->clones[j] << " ";
        }
        std::cout << '\n';
        std::cout << "  Modes (" << output_info->nmode << "):\n";
        for (int j = 0; j < output_info->nmode ; j++) {
            auto mode = output_info->modes[j];
            auto mode_info = mode_infos[mode];
            std::cout << "  - Mode #" << j << '\n';
            std::cout << "    Name: " << mode_info.name << '\n';
            std::cout << "    Resolution: " << mode_info.width << " x " << mode_info.height << '\n';
            std::cout << "    Refresh Rate: " << kat::window::x11::calcRefreshRate(mode_info) << '\n';
        }
    }

    auto dpi = windowing_engine->platform->dpi();
    auto scale = windowing_engine->platform->scale();


    std::cout << "System DPI: " << dpi.x << " x " << dpi.y << '\n';
    std::cout << "Scale: " << scale.x << " x " << scale.y << '\n';

    return EXIT_SUCCESS;
}