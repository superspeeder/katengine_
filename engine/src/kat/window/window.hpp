#pragma once

#include <memory>
#include "kat/cfg.hpp"
#include <concepts>

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


    /**
     * Monitor Interface API
     *
     * The monitor type for the target platform **must** implement:
     * - glm::vec2 dpi() const
     * - glm::vec2 scale() const
     * - glm::uvec2 physical_size() const
     * - glm::uvec2 size() const
     * - std::string_view name() const
     * - bool is_primary() const
     * - video_mode video_mode() const
     * - std::vector<video_mode> video_modes()
     */
#ifdef KAT_PLATFORM_VERIFYINTERFACES
    namespace {
        template<typename T>
        concept is_monitor = requires(const T &value) {
            { value.dpi() } -> std::same_as<glm::vec2>;
            { value.scale() } -> std::same_as<glm::vec2>;
            { value.physical_size() } -> std::same_as<glm::uvec2>;
            { value.size() } -> std::same_as<glm::uvec2>;
            { value.name() } -> std::same_as<std::string_view>;
            { value.is_primary() } -> std::same_as<bool>;
            { value.video_mode() } -> std::same_as<window::video_mode>;
            { value.video_modes() } -> std::same_as<std::vector<window::video_mode>>;
        };

        static_assert(is_monitor<monitor>, "monitor interface not implemented correctly.");
    }
#endif
}