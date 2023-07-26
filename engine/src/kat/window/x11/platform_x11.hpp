#pragma once

#include "kat/cfg.hpp"

#ifdef KATWINDOW_TARGET_X11

#include "kat/window/utils.hpp"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <vector>
#include <glm/glm.hpp>
#include <string_view>
#include <memory>

namespace kat::window {
    struct windowing_engine;

    namespace x11 {
        struct engine_state_x11 {
            Display* display;
            int screen_id;
            Screen* screen;
            Window root;

            engine_state_x11();
            ~engine_state_x11();

            glm::vec2 dpi();
            glm::vec2 scale();
        };

        int calcRefreshRate(const XRRModeInfo& modeInfo);

        class monitor_x11 {
        public:
            monitor_x11(const std::shared_ptr<windowing_engine>& engine, const XRRMonitorInfo &monitor_info_);

            [[maybe_unused]] [[nodiscard]] const XRRMonitorInfo &monitor_info() const;

            [[nodiscard]] glm::vec2 dpi() const;
            [[nodiscard]] glm::vec2 scale() const;

            [[nodiscard]] glm::uvec2 physical_size() const;
            [[nodiscard]] glm::uvec2 size() const;
            [[nodiscard]] glm::ivec2 position() const;

            [[nodiscard]] std::string_view name() const;

            [[nodiscard]] bool is_primary() const;

            const std::vector<RROutput>& outputs() const noexcept;

        private:
            XRRMonitorInfo m_monitor_info;
            std::shared_ptr<windowing_engine> m_windowing_engine;
            std::vector<RROutput> m_outputs;
        };

        class window_x11 {
        public:

        private:

            glm::vec2 dpi();
            glm::vec2 scale();

            glm::uvec2 size();
            glm::ivec2 position();
        };
    }

    using platform_state = x11::engine_state_x11;
    using monitor = x11::monitor_x11;
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine>& engine);
}

#endif