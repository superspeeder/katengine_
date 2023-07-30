#pragma once
#include "kat/cfg.hpp"
#include "kat/window/utils.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <string_view>
#include <string>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <shellscalingapi.h>
#include <dwmapi.h>
#include <Windows.h>
#include <windef.h>
#include <WinUser.h>


#ifdef KATWINDOW_TARGET_WIN32

namespace kat::window {
    struct windowing_engine;

    namespace win32 {
        class monitor_win32;

        struct engine_state_win32 {
            std::vector<std::shared_ptr<monitor_win32>> m_monitors;

            engine_state_win32();

            [[nodiscard]] std::vector<std::shared_ptr<monitor_win32>> monitors() const;
            void setup(const std::shared_ptr<windowing_engine>& engine);
        };

        class monitor_win32 {
        public:
            monitor_win32(const DISPLAY_DEVICE &adapter, const DISPLAY_DEVICE &display, const std::shared_ptr<windowing_engine>& engine);

            [[nodiscard]] glm::vec2 dpi() const;
            [[nodiscard]] glm::vec2 scale() const;

            [[nodiscard]] glm::uvec2 physical_size() const;
            [[nodiscard]] glm::uvec2 size() const;
            [[nodiscard]] glm::ivec2 position() const;

            [[nodiscard]] std::string_view name() const;

            [[nodiscard]] bool is_primary() const;

            [[nodiscard]] kat::window::video_mode video_mode() const;
            [[nodiscard]] std::vector<kat::window::video_mode> video_modes() const;

        private:

            HMONITOR m_handle;
            glm::uvec2 m_physical_size, m_size;
            glm::ivec2 m_position;
            std::vector<kat::window::video_mode> m_video_modes;
            bool m_is_primary;
            glm::uvec2 m_dpi;

            std::shared_ptr<windowing_engine> m_windowing_engine;
            window::video_mode m_video_mode;

            std::string m_display_name, m_adapter_name;
            std::string m_display_readable_name, m_adapter_readable_name;

        };

        class window_win32 {
        public:

            window_win32(/* TODO */);

        private:

            HWND m_hwnd;
        };
    }


    using platform_state = win32::engine_state_win32;
    using monitor = win32::monitor_win32;
    using window = win32::window_win32;
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine>& engine);
}

#endif