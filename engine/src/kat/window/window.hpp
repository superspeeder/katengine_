#pragma once

#include <memory>
#include "kat/cfg.hpp"

#ifdef KATWINDOW_TARGET_X11
#include "kat/window/x11/platform_x11.hpp"
#elif defined(KATWINDOW_TARGET_WIN32)
#include "kat/window/win32/platform_win32.hpp"
#endif

namespace kat::window {
    struct windowing_engine : public std::enable_shared_from_this<windowing_engine> {
        kat::window::platform_state* platform;

        [[nodiscard]] std::vector<std::shared_ptr<monitor>> monitors();

        ~windowing_engine();

        [[nodiscard]] static inline std::shared_ptr<windowing_engine> create() { return std::shared_ptr<windowing_engine>(new windowing_engine()); };

    private:
        explicit windowing_engine();

    };

}