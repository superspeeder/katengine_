#include "platform_x11.hpp"
#include "kat/window/window.hpp"
#include <spdlog/spdlog.h>
#include <X11/Xresource.h>

namespace kat::window::x11 {
    engine_state_x11::engine_state_x11() {
        display = XOpenDisplay(nullptr);
        screen_id = DefaultScreen(display);
        screen = ScreenOfDisplay(display, screen_id);
        root = RootWindowOfScreen(screen);

        SPDLOG_DEBUG("Opened Display {}", XDisplayString(display));
        SPDLOG_DEBUG("Using Default Screen (#{})", screen_id);
        SPDLOG_DEBUG("Screen Virtual Size: {} x {}", screen->width, screen->height);
    }

    engine_state_x11::~engine_state_x11() {
        XCloseDisplay(display);
        SPDLOG_DEBUG("Closed Display");
    }

    glm::vec2 engine_state_x11::dpi() {
        float x, y;
        x = y = KAT_BASE_DPI;
        char* rms = XResourceManagerString(display);
        if (rms) {
            XrmDatabase db = XrmGetStringDatabase(rms);
            if (db) {
                XrmValue value;
                char* type = nullptr;
                if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
                    if (type && strcmp(type, "String") == 0) {
                        x = y = std::atof(value.addr);
                    }
                }

                XrmDestroyDatabase(db);
            }
        }

        // if other ways to get dpi exist, maybe use those.

        return {x, y};
    }

    glm::vec2 engine_state_x11::scale() {
        return kat::window::conv_dpi_to_scale(dpi());
    }

    monitor_x11::monitor_x11(const std::shared_ptr<windowing_engine>& engine, const XRRMonitorInfo &monitor_info_) : m_windowing_engine(engine), m_monitor_info(monitor_info_) {
        m_outputs.resize(m_monitor_info.noutput);
        std::memcpy(m_outputs.data(), m_monitor_info.outputs, m_monitor_info.noutput * sizeof(RROutput));
    }

    const XRRMonitorInfo &monitor_x11::monitor_info() const {
        return m_monitor_info;
    }

    glm::vec2 monitor_x11::dpi() const {
        return m_windowing_engine->platform->dpi();
    }

    glm::vec2 monitor_x11::scale() const {
        return kat::window::conv_dpi_to_scale(dpi());
    }

    glm::uvec2 monitor_x11::physical_size() const {
        return { m_monitor_info.mwidth, m_monitor_info.mheight };
    }

    glm::uvec2 monitor_x11::size() const {
        return { m_monitor_info.width, m_monitor_info.height };
    }

    glm::ivec2 monitor_x11::position() const {
        return { m_monitor_info.x, m_monitor_info.y };
    }

    std::string_view monitor_x11::name() const {
        return "";
    }

    bool monitor_x11::is_primary() const {
        return m_monitor_info.primary;
    }

    const std::vector<RROutput> &monitor_x11::outputs() const noexcept {
        return m_outputs;
    }

    int calcRefreshRate(const XRRModeInfo &modeInfo) {
        if (modeInfo.hTotal != 0 && modeInfo.vTotal != 0) {
            return static_cast<int>(round(static_cast<double>(modeInfo.dotClock) / static_cast<double>(modeInfo.hTotal * modeInfo.vTotal)));
        } else {
            return 0;
        }
    }
}

namespace kat::window {
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine> &engine) {
        int count;
        XRRMonitorInfo* monitorInfos = XRRGetMonitors(engine->platform->display, engine->platform->root, 0, &count);

        std::vector<std::shared_ptr<monitor>> monitors;
        monitors.resize(count);

        for (int i = 0 ; i < count ; i++) {
            monitors[i] = std::make_shared<monitor>(engine, monitorInfos[i]);
        }

        XRRFreeMonitors(monitorInfos);

        return monitors;
    }
}