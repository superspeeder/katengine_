#include "kat/cfg.hpp"

#ifdef KATWINDOW_TARGET_WIN32
#include "platform_win32.hpp"
#include "kat/window/window.hpp"
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

        mep_params params{};
        params.pHandle = &m_handle;
        params.pAdapterName = adapter.DeviceName;

        m_display_readable_name = display.DeviceString;
        m_adapter_readable_name = display.DeviceString;

        RECT rect{};
        rect.left = m_position.x;
        rect.top = m_position.y;
        rect.right = m_position.x + devMode.dmPelsWidth;
        rect.bottom = m_position.y + devMode.dmPelsHeight;

        EnumDisplayMonitors(nullptr, &rect, monitor_enum_proc, reinterpret_cast<LPARAM>(&params));

        GetDpiForMonitor(m_handle, MDT_EFFECTIVE_DPI, &m_dpi.x, &m_dpi.y);

        m_size.x /= m_dpi.x / 96;
        m_size.y /= m_dpi.y / 96;

        m_is_primary = m_position == glm::ivec2(0, 0);
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



    win32::engine_state_win32::engine_state_win32() {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }

    std::vector<std::shared_ptr<monitor>> win32::engine_state_win32::monitors() const {
        return m_monitors;
    }

    LRESULT CALLBACK wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_CREATE: {
                auto *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
                SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
                return 0;
            }
        }

        LONG_PTR lpUserData = GetWindowLongPtrA(hWnd, GWLP_USERDATA);
        if (lpUserData) {
            auto* w = reinterpret_cast<window*>(lpUserData);
            return w->window_proc(hWnd, uMsg, wParam, lParam);
        }


        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

    const char* const wc_name = "katwc";

    void win32::engine_state_win32::setup(const std::shared_ptr<windowing_engine> &engine) {
        m_instance = GetModuleHandleA(nullptr);

        m_monitors = get_all_monitors(engine);

        WNDCLASSEXA wc;
        ZeroMemory(&wc, sizeof(WNDCLASSEXA));
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.hInstance = m_instance;
        wc.lpfnWndProc = wndproc;
        wc.lpszClassName = wc_name;
        wc.style = CS_HREDRAW | CS_VREDRAW;

        RegisterClassExA(&wc);
    }

    void win32::engine_state_win32::process_events() {
        MSG msg{};
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_app_exit = true;
                SPDLOG_INFO("Exit");
                break;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

    }

    bool win32::engine_state_win32::is_app_exit() const {
        return m_app_exit;
    }

    win32::window_win32::window_win32(const std::shared_ptr<kat::window::windowing_engine> &engine,
                                      const std::string_view title, const glm::uvec2 &size, const glm::ivec2 &position) : m_windowing_engine(engine) {
        m_hwnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, wc_name, title.data(), WS_OVERLAPPEDWINDOW, position.x, position.y, size.x, size.y, nullptr, nullptr /* TODO: maybe support idk */, engine->platform->m_instance, this);
        ShowWindow(m_hwnd, SW_NORMAL);
    }

    win32::window_win32::~window_win32() {
        DestroyWindow(m_hwnd);
    }

    std::string win32::window_win32::title() const {
        auto length = GetWindowTextLengthA(m_hwnd) + 1;
        std::string text(length, '\0');

        GetWindowTextA(m_hwnd, text.data(), length);
        return text;
    }

    void win32::window_win32::title(const std::string_view new_title) {
        SetWindowTextA(m_hwnd, new_title.data());
    }

    glm::vec2 win32::window_win32::dpi() const {
        UINT dpi = GetDpiForWindow(m_hwnd);
        return { dpi, dpi };
    }

    glm::vec2 win32::window_win32::scale() const {
        return dpi() / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
    }

    glm::uvec2 win32::window_win32::size() const {
        RECT r;
        ZeroMemory(&r, sizeof(RECT));
        GetClientRect(m_hwnd, &r);
        return { r.right - r.left, r.bottom - r.top };
    }

    glm::ivec2 win32::window_win32::position() const {
        RECT r;
        ZeroMemory(&r, sizeof(RECT));
        GetWindowRect(m_hwnd, &r);
        return { r.left, r.top };
    }

    void win32::window_win32::size(glm::uvec2 new_size) {
        RECT r;
        ZeroMemory(&r, sizeof(RECT));
        GetWindowRect(m_hwnd, &r);
        RECT r2 = { r.left, r.top, static_cast<LONG>(r.left + new_size.x), static_cast<LONG>(r.top + new_size.y) };
        LONG s, s_ex;
        s = GetWindowLong(m_hwnd, GWL_STYLE);
        s_ex = GetWindowLong(m_hwnd, GWL_EXSTYLE);
        AdjustWindowRectExForDpi(&r2, s, m_menu != nullptr, s_ex, GetDpiForWindow(m_hwnd));

        SetWindowPos(m_hwnd, HWND_TOP, r.left, r.top, r2.right - r2.left, r2.bottom - r2.top, 0);

    }

    void win32::window_win32::position(glm::ivec2 new_position) {
        RECT r;
        ZeroMemory(&r, sizeof(RECT));
        GetWindowRect(m_hwnd, &r);

        MoveWindow(m_hwnd, new_position.x, new_position.y, r.right - r.left, r.bottom - r.top, false); // no need to repaint since we don't support GDI
    }

    bool win32::window_win32::decorated() const {
        return m_decorated;
    }

    void win32::window_win32::decorated(bool new_mode) {
        if (m_decorated ^ new_mode) {
            m_decorated = new_mode;
            if (m_decorated) {
                DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
                SetWindowLong(m_hwnd, GWL_STYLE, style | WS_POPUP);
            } else {
                DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
                SetWindowLong(m_hwnd, GWL_STYLE, style & (~WS_POPUP));
            }
        }
    }

    void win32::window_win32::restore() {
        ShowWindow(m_hwnd, SW_RESTORE);
    }

    void win32::window_win32::maximize() {
        ShowWindow(m_hwnd, SW_MAXIMIZE);
    }

    void win32::window_win32::minimize() {
        ShowWindow(m_hwnd, SW_MINIMIZE);
    }

    void win32::window_win32::show() {
        ShowWindow(m_hwnd, SW_NORMAL);
    }

    void win32::window_win32::hide() {
        ShowWindow(m_hwnd, SW_HIDE);
    }

    HWND win32::window_win32::platform_handle() const {
        return m_hwnd;
    }

    LRESULT win32::window_win32::window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_CLOSE:
                DestroyWindow(hWnd);
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
        }

        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
}

#endif