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

        [[nodiscard]] std::vector<std::shared_ptr<monitor>> monitors() const;

        ~windowing_engine();

        [[nodiscard]] static inline std::shared_ptr<windowing_engine> create() {
            auto sp = std::shared_ptr<windowing_engine>(new windowing_engine());
            sp->platform->setup(sp);
            return sp;
        };

        void process_events() const;

        bool is_app_exit() const;

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
            { value.position() } -> std::same_as<glm::ivec2>;
            { value.physical_size() } -> std::same_as<glm::uvec2>;
            { value.size() } -> std::same_as<glm::uvec2>;
            { value.name() } -> std::same_as<std::string_view>;
            { value.is_primary() } -> std::same_as<bool>;
            { value.video_mode() } -> std::same_as<::kat::window::video_mode>;
            { value.video_modes() } -> std::same_as<std::vector<::kat::window::video_mode>>;
        };

        template<typename T>
        concept is_window = requires(const T &value) {
            { value.dpi() } -> std::same_as<glm::vec2>;
            { value.scale() } -> std::same_as<glm::vec2>;
            { value.position() } -> std::same_as<glm::ivec2>;
            { value.size() } -> std::same_as<glm::uvec2>;
            { value.title() } -> std::same_as<std::string>;
            { value.decorated() } -> std::same_as<bool>;
        } && requires(T& value, glm::uvec2 new_uvec2, glm::ivec2 new_ivec2, std::string_view new_string, bool new_bool) {
            { value.size(new_uvec2) } -> std::same_as<void>;
            { value.position(new_ivec2) } -> std::same_as<void>;
            { value.title(new_string) } -> std::same_as<void>;
            { value.decorated(new_bool) } -> std::same_as<void>;
            { value.restore() } -> std::same_as<void>;
            { value.maximize() } -> std::same_as<void>;
            { value.minimize() } -> std::same_as<void>;
            { value.show() } -> std::same_as<void>;
            { value.hide() } -> std::same_as<void>;
        };

        template<typename T>
        concept is_platform_state = requires(T& value, const std::shared_ptr<windowing_engine>& engine) {
            { value.setup(engine) } -> std::same_as<void>;
            { value.process_events() } -> std::same_as<void>;
        } && requires(const T& value) {
            { value.monitors() } -> std::same_as<std::vector<std::shared_ptr<monitor>>>;
            { value.is_app_exit() } -> std::same_as<bool>;
        };

        static_assert(is_monitor<monitor>, "monitor interface not implemented correctly.");
        static_assert(is_window<window>, "window interface not implemented correctly.");
        static_assert(is_platform_state<platform_state>, "platform_state interface not implemented correctly.");
    }
#endif
}