#include "kat/cfg.hpp"
#include "kat/cfg.hpp"

#ifdef KATWINDOW_TARGET_WIN32
#include "platform_win32.hpp"
#include "spdlog/spdlog.h"

namespace kat::window {

    display_depth make_display_depth_win32(DWORD depth) {
        if (depth == 32) depth = 24;
        display_depth dd{};
        dd.red = dd.green = dd.blue = depth / 3;
        int delta = depth % 3;
        if (delta >= 1) {
            dd.green += 1;
        }

        if (delta == 2) {
            dd.red += 1;
        }

        return dd;
    }

    video_mode create_video_mode(const DEVMODEA& devMode) {
        return {
                { devMode.dmPelsWidth, devMode.dmPelsHeight },
                static_cast<int>(devMode.dmDisplayFrequency),
                make_display_depth_win32(devMode.dmBitsPerPel)
        };
    }

    struct mep_params {
        HMONITOR* pHandle;
        const char* pAdapterName;
    };

    BOOL CALLBACK monitor_enum_proc(HMONITOR hMonitor, HDC hDc, LPRECT lpRect, LPARAM lpUserData) {
        auto* pParams = reinterpret_cast<mep_params*>(lpUserData);
        MONITORINFOEXA mi;
        mi.cbSize = sizeof(MONITORINFOEXA);
        if (GetMonitorInfoA(hMonitor, &mi)) {
            SPDLOG_INFO("{} -- {}", mi.szDevice, pParams->pAdapterName);
            if (strcmp(mi.szDevice, pParams->pAdapterName) == 0) {
                *(pParams->pHandle) = hMonitor;
            }
        }

        return true;
    }

    win32::monitor_win32::monitor_win32(const DISPLAY_DEVICE &adapter, const DISPLAY_DEVICE &display, const std::shared_ptr<windowing_engine>& engine) : m_windowing_engine(engine) {
        m_display_name = display.DeviceName;
        m_adapter_name = adapter.DeviceName;

        SPDLOG_INFO("{}", m_adapter_name);

        HDC hdc = CreateDCA("DISPLAY", m_adapter_name.c_str(), nullptr, nullptr);
        m_physical_size.x = GetDeviceCaps(hdc, HORZSIZE);
        m_physical_size.y = GetDeviceCaps(hdc, VERTSIZE);
        m_size.x = GetDeviceCaps(hdc, HORZRES);
        m_size.y = GetDeviceCaps(hdc, VERTRES);
        DeleteDC(hdc);

        int i = 0;
        DEVMODEA devMode;
        ZeroMemory(&devMode, sizeof(DEVMODEA));
        while (EnumDisplaySettingsA(adapter.DeviceName, i++, &devMode)) {
            m_video_modes.push_back(create_video_mode(devMode));
            ZeroMemory(&devMode, sizeof(DEVMODEA));
        }

        EnumDisplaySettingsA(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
        m_video_mode = create_video_mode(devMode);

        m_position.x = devMode.dmPosition.x;
        m_position.y = devMode.dmPosition.y;

        RECT rect{};
        rect.left = m_position.x;
        rect.top = m_position.y;
        rect.right = m_position.x + devMode.dmPelsWidth;
        rect.bottom = m_position.y + devMode.dmPelsHeight;

        mep_params params{};
        params.pHandle = &m_handle;
        params.pAdapterName = adapter.DeviceName;

        m_display_readable_name = display.DeviceString;
        m_adapter_readable_name = display.DeviceString;

        EnumDisplayMonitors(nullptr, &rect, monitor_enum_proc, reinterpret_cast<LPARAM>(&params));

        GetDpiForMonitor(m_handle, MDT_EFFECTIVE_DPI, &m_dpi.x, &m_dpi.y);

        m_size.x /= m_dpi.x / 96;
        m_size.y /= m_dpi.y / 96;

        m_is_primary = m_position == glm::ivec2(0, 0);
    }

    glm::vec2 win32::monitor_win32::dpi() const {
        return m_dpi;
    }

    glm::vec2 win32::monitor_win32::scale() const {
        return { static_cast<float>(m_dpi.x) / USER_DEFAULT_SCREEN_DPI, static_cast<float>(m_dpi.y) / USER_DEFAULT_SCREEN_DPI };
    }

    glm::uvec2 win32::monitor_win32::physical_size() const {
        return m_physical_size;
    }

    glm::uvec2 win32::monitor_win32::size() const {
        return m_size;
    }

    glm::ivec2 win32::monitor_win32::position() const {
        return m_position;
    }

    std::string_view win32::monitor_win32::name() const {
        return m_display_readable_name;
    }

    bool win32::monitor_win32::is_primary() const {
        return m_is_primary;
    }

    kat::window::video_mode win32::monitor_win32::video_mode() const {
        return m_video_mode;
    }

    std::vector<kat::window::video_mode> win32::monitor_win32::video_modes() const {
        return m_video_modes;
    }

    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine> &engine) {
        std::vector<std::shared_ptr<monitor>> monitors;
        DISPLAY_DEVICEA adapter;
        adapter.cb = sizeof(DISPLAY_DEVICEA);
        int i = 0;

        while (EnumDisplayDevicesA(nullptr, i, &adapter, 0)) {
            SPDLOG_INFO("Adapter: {} ; {}", adapter.DeviceName, adapter.DeviceString);

            i += 1;
            if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
                SPDLOG_INFO("{} Inactive", adapter.DeviceName);
                continue;
            }

            if (adapter.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                SPDLOG_INFO("{} Desktop", adapter.DeviceName);
                DISPLAY_DEVICEA display;
                display.cb = sizeof(DISPLAY_DEVICEA);
                int j = 0;
                while (EnumDisplayDevicesA(adapter.DeviceName, j, &display, 0)) {
                    SPDLOG_INFO("Display: {}", display.DeviceName);
                    j += 1;
                    if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
                        SPDLOG_INFO("{} Inactive", display.DeviceName);
                    } else {
                        monitors.push_back(std::make_shared<monitor>(adapter, display, engine));
                    }
                    ZeroMemory(&display, sizeof(DISPLAY_DEVICEA));
                    display.cb = sizeof(DISPLAY_DEVICEA);
                }

                if (j == 0) {
                    SPDLOG_INFO("{} Inactive");
                    monitors.push_back(std::make_shared<monitor>(adapter, adapter, engine)); // im the monitor now
                }
            }

            ZeroMemory(&adapter, sizeof(DISPLAY_DEVICEA));
            adapter.cb = sizeof(DISPLAY_DEVICEA);
        }

        return monitors;
    }

    win32::engine_state_win32::engine_state_win32() {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }

    std::vector<std::shared_ptr<monitor>> win32::engine_state_win32::monitors() const {
        return m_monitors;
    }

    void win32::engine_state_win32::setup(const std::shared_ptr<windowing_engine> &engine) {
        m_monitors = get_all_monitors(engine);
    }
}

#endif