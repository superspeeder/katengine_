#include "kat/window/window.hpp"
#include <spdlog/spdlog.h>

namespace kat::window {
    windowing_engine::windowing_engine() : platform(new platform_state()) {
    }

    windowing_engine::~windowing_engine() {
        delete platform;
    }

    std::vector<std::shared_ptr<monitor>> windowing_engine::monitors() const {
        return platform->monitors();
    }

    void windowing_engine::process_events() const {
        platform->process_events();
    }

    bool windowing_engine::is_app_exit() const {
        return platform->is_app_exit();
    }
}


