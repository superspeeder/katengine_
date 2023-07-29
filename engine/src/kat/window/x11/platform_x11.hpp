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
#include <string>
#include <unordered_map>

namespace kat::window {
    struct windowing_engine;

    namespace x11 {
        video_mode make_video_mode_x11(const XRRModeInfo& mode_info, const XRRCrtcInfo* crtc_info, Screen* screen);

        display_depth make_display_depth_x11(int depth);

        struct engine_state_x11 {
            Display* display;
            int screen_id;
            Screen* screen;
            Window root;
            std::unordered_map<RRMode, XRRModeInfo> mode_infos;
            XRRScreenResources* scr_res;

            engine_state_x11();
            ~engine_state_x11();

            [[nodiscard]] glm::vec2 dpi();
            [[nodiscard]] glm::vec2 scale();
        };

        int calc_refresh_rate(const XRRModeInfo& modeInfo);

        class monitor_x11 {
        public:
            monitor_x11(const std::shared_ptr<windowing_engine>& engine, const XRRMonitorInfo &monitor_info, const XRROutputInfo& output_info, RROutput output);

            [[nodiscard]] glm::vec2 dpi() const;
            [[nodiscard]] glm::vec2 scale() const;

            [[nodiscard]] glm::uvec2 physical_size() const;
            [[nodiscard]] glm::uvec2 size() const;
            [[nodiscard]] glm::ivec2 position() const;

            [[nodiscard]] std::string_view name() const;

            [[nodiscard]] bool is_primary() const;

            [[nodiscard]] kat::window::video_mode video_mode() const;
            [[nodiscard]] std::vector<kat::window::video_mode> video_modes() const;


            [[nodiscard]] RROutput get_output() const;

        private:
            RROutput m_output;
            glm::uvec2 m_physical_size, m_size;
            glm::ivec2 m_position;
            std::string m_name;
            std::vector<kat::window::video_mode> m_video_modes;
            RRCrtc m_crtc;
            bool m_is_primary;
            Atom m_monitor_idname;

            std::shared_ptr<windowing_engine> m_windowing_engine;
            window::video_mode m_video_mode;
        };

        class window_x11 {
        public:

            window_x11(const std::shared_ptr<windowing_engine>& engine, std::string_view title_, glm::uvec2 size_, glm::uvec2 position_);
            ~window_x11();

            std::string_view title() const;
            void title(std::string_view new_title);

            glm::vec2 dpi() const;
            glm::vec2 scale() const;

            glm::uvec2 size() const;
            glm::ivec2 position() const;

            void size(glm::uvec2 new_size);
            void position(glm::uvec2 new_position);

            glm::uvec2 client_size() const;
            void client_size(glm::uvec2 new_client_size) const;

            Window platform_handle() const;



        private:
            Window m_window;
            std::shared_ptr<windowing_engine> m_windowing_engine;
        };
    }

    using platform_state = x11::engine_state_x11;
    using monitor = x11::monitor_x11;
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine>& engine);
}

#endif