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
            HINSTANCE m_instance;

            engine_state_win32();

            [[nodiscard]] std::vector<std::shared_ptr<monitor_win32>> monitors() const;
            void setup(const std::shared_ptr<windowing_engine>& engine);

            void process_events();
            bool is_app_exit() const;

            bool m_app_exit = false;
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

            window_win32(const std::shared_ptr<kat::window::windowing_engine>& engine, std::string_view title, const glm::uvec2 &size, const glm::ivec2 &position);
            ~window_win32();

            [[nodiscard]] std::string title() const;
            void title(std::string_view new_title);

            [[nodiscard]] glm::vec2 dpi() const;
            [[nodiscard]] glm::vec2 scale() const;

            [[nodiscard]] glm::uvec2 size() const;
            [[nodiscard]] glm::ivec2 position() const;

            void size(glm::uvec2 new_size);
            void position(glm::ivec2 new_position);

            [[nodiscard]] bool decorated() const;
            void decorated(bool new_mode);

            void restore();
            void maximize();
            void minimize();

            void show();
            void hide();

            [[nodiscard]] HWND platform_handle() const;

            LRESULT window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        private:
            std::shared_ptr<kat::window::windowing_engine> m_windowing_engine;
            HMENU m_menu = nullptr;
            HWND m_hwnd;
            bool m_decorated = true;
        };
    }


    using platform_state = win32::engine_state_win32;
    using monitor = win32::monitor_win32;
    using window = win32::window_win32;
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine>& engine);
}

#endif