#include "kat/window/window.hpp"
#include <spdlog/spdlog.h>

namespace kat::window {
    windowing_engine::windowing_engine() : platform(new platform_state()) {
    }

    windowing_engine::~windowing_engine() {
        delete platform;
    }

    std::vector<std::shared_ptr<monitor>> windowing_engine::monitors() {
        return get_all_monitors(shared_from_this());
    }
}


