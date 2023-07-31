#include "kat/cfg.hpp"
#ifdef KATWINDOW_TARGET_X11
#include "platform_x11.hpp"
#include "kat/window/window.hpp"
#include <spdlog/spdlog.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <set>
#include <unordered_map>
#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#include <Xm/MwmUtil.h>

namespace kat::window::x11 {
    engine_state_x11::engine_state_x11() {
        display = XOpenDisplay(nullptr);
        screen_id = DefaultScreen(display);
        screen = ScreenOfDisplay(display, screen_id);
        root = RootWindowOfScreen(screen);

        scr_res = XRRGetScreenResources(display, root);

        for (int i = 0 ; i < scr_res->nmode ; i++) {
            auto modei = scr_res->modes[i];
            mode_infos[modei.id] = modei;
        }

        SPDLOG_DEBUG("Opened Display {}", XDisplayString(display));
        SPDLOG_DEBUG("Using Default Screen (#{})", screen_id);
        SPDLOG_DEBUG("Screen Virtual Size: {} x {}", screen->width, screen->height);
    }

    engine_state_x11::~engine_state_x11() {
        XRRFreeScreenResources(scr_res);
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

    void engine_state_x11::setup(const std::shared_ptr<windowing_engine> &engine) {
        m_monitors = get_all_monitors(engine);
    }

    std::vector<std::shared_ptr<monitor_x11>> engine_state_x11::monitors() const {
        return m_monitors;
    }

    monitor_x11::monitor_x11(const std::shared_ptr<windowing_engine>& engine, const XRRMonitorInfo &monitor_info, const XRROutputInfo& output_info, RROutput output) : m_output(output), m_windowing_engine(engine) {
        m_size = { monitor_info.width, monitor_info.height };
        m_position = { monitor_info.x, monitor_info.y };
        m_name = output_info.name;
        m_physical_size = { output_info.mm_width, output_info.mm_height };
        m_crtc = output_info.crtc;
        m_is_primary = monitor_info.primary;
        m_monitor_idname = monitor_info.name;
        m_video_modes.resize(output_info.nmode);
        auto* crtc_info = XRRGetCrtcInfo(engine->platform->display, engine->platform->scr_res, m_crtc);

        for (int i = 0 ; i < output_info.nmode ; i++) {
            auto mode = output_info.modes[i];
            XRRModeInfo modeinfo = engine->platform->mode_infos[mode];
            m_video_modes[i] = make_video_mode_x11(modeinfo, crtc_info, engine->platform->screen);
        }

        RRMode mode = crtc_info->mode;
        XRRModeInfo modeinfo = engine->platform->mode_infos[mode];
        kat::window::video_mode vm = make_video_mode_x11(modeinfo, crtc_info, engine->platform->screen);
        m_video_mode = vm;
    }

    glm::vec2 monitor_x11::dpi() const {
        return m_windowing_engine->platform->dpi();
    }

    glm::vec2 monitor_x11::scale() const {
        return kat::window::conv_dpi_to_scale(dpi());
    }

    glm::uvec2 monitor_x11::physical_size() const {
        return m_physical_size;
    }

    glm::uvec2 monitor_x11::size() const {
        return m_size;
    }

    glm::ivec2 monitor_x11::position() const {
        return m_position;
    }

    std::string_view monitor_x11::name() const {
        return m_name;
    }

    bool monitor_x11::is_primary() const {
        return m_is_primary;
    }

    RROutput monitor_x11::get_output() const {
        return m_output;
    }

    ::kat::window::video_mode monitor_x11::video_mode() const {
        return m_video_mode;
    }

    std::vector<::kat::window::video_mode> monitor_x11::video_modes() const {
        return m_video_modes;
    }

    int calc_refresh_rate(const XRRModeInfo &modeInfo) {
        if (modeInfo.hTotal != 0 && modeInfo.vTotal != 0) {
            return static_cast<int>(round(static_cast<double>(modeInfo.dotClock) / static_cast<double>(modeInfo.hTotal * modeInfo.vTotal)));
        } else {
            return 0;
        }
    }

    ::kat::window::video_mode make_video_mode_x11(const XRRModeInfo &mode_info, const XRRCrtcInfo* crtc_info, Screen* screen) {
        ::kat::window::video_mode mode{};

        mode.refresh_rate = calc_refresh_rate(mode_info);
        mode.resolution = { mode_info.width, mode_info.height };
        mode.depth = make_display_depth_x11(DefaultDepthOfScreen(screen));

        return mode;
    }

    display_depth make_display_depth_x11(int depth) {
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
}

namespace kat::window {
    std::vector<std::shared_ptr<monitor>> get_all_monitors(const std::shared_ptr<windowing_engine> &engine) {
        int count;
        XRRMonitorInfo* monitorInfos = XRRGetMonitors(engine->platform->display, engine->platform->root, 0, &count);

        std::vector<std::shared_ptr<monitor>> monitors;

        std::unordered_map<RROutput, XRRMonitorInfo> active_outputs;

        for (int i = 0 ; i < count ; i++) {
            for (int j = 0 ; j < monitorInfos[i].noutput ; j++) {
                active_outputs[monitorInfos[i].outputs[j]] = monitorInfos[i];
            }
        }

        auto sr = XRRGetScreenResources(engine->platform->display, engine->platform->root);

        for (int i = 0 ; i < sr->noutput ; i++) {
            auto output = sr->outputs[i];
            auto output_info = XRRGetOutputInfo(engine->platform->display, sr, output);
            if (output_info->connection != RR_Disconnected && active_outputs.find(output) != active_outputs.end()) {
                auto monitor_info = active_outputs[output];
                monitors.push_back(std::make_shared<monitor>(engine, monitor_info, *output_info, output));
            }

            XRRFreeOutputInfo(output_info);
        }


        XRRFreeScreenResources(sr);
        XRRFreeMonitors(monitorInfos);

        return monitors;
    }

    x11::window_x11::window_x11(const std::shared_ptr<windowing_engine>& engine, std::string_view title_, glm::uvec2 size_, glm::uvec2 position_) : m_windowing_engine(engine) {
        Cursor cursor = XCreateFontCursor(engine->platform->display, XC_left_side);

        XSetWindowAttributes swa{};
        swa.colormap = engine->platform->screen->cmap;
        swa.event_mask = StructureNotifyMask | KeyPressMask | ExposureMask | ControlMask;
        swa.cursor = cursor;

        m_window = XCreateWindow(m_windowing_engine->platform->display, m_windowing_engine->platform->root,
                      position_.x, position_.y, size_.x, size_.y, 0,
                      CopyFromParent, InputOutput, CopyFromParent,
                      CWEventMask | CWColormap | CWCursor,
                      &swa);



        XStoreName(engine->platform->display, m_window, title_.data());
        XMapWindow(engine->platform->display, m_window);

//        //code to remove decoration
//        PropMwmHints hints;
//        Atom property;
//        hints.flags = MWM_HINTS_DECORATIONS;
//        hints.decorations = 0;
//        property = XInternAtom(m_windowing_engine->platform->display, _XA_MWM_HINTS, true);
//        XChangeProperty(m_windowing_engine->platform->display,m_window,property,property,32,PropModeReplace,(unsigned char *)&hints,5);

        XMapWindow(m_windowing_engine->platform->display, m_window);


    }

    x11::window_x11::~window_x11() {

    }

    std::string_view x11::window_x11::title() const {
        return std::string_view();
    }

    void x11::window_x11::title(std::string_view new_title) {

    }

    glm::vec2 x11::window_x11::dpi() const {
        return glm::vec2();
    }

    glm::vec2 x11::window_x11::scale() const {
        return glm::vec2();
    }

    glm::uvec2 x11::window_x11::size() const {
        XWindowAttributes wa;
        XGetWindowAttributes(m_windowing_engine->platform->display, m_window, &wa);

        return { wa.width, wa.height };
    }

    glm::ivec2 x11::window_x11::position() const {
        Window root;
        int x, y;
        unsigned int w, h, bw, d;
        XTranslateCoordinates(m_windowing_engine->platform->display, m_window, m_windowing_engine->platform->root, 0, 0, &x, &y, &root);

        return { x, y };
    }

    void x11::window_x11::size(glm::uvec2 new_size) {
        XMoveWindow(m_windowing_engine->platform->display, m_window, new_size.x, new_size.y);
        XFlush(m_windowing_engine->platform->display);
    }

    void x11::window_x11::position(glm::ivec2 new_position) {
        XWindowChanges changes{};
        changes.x = new_position.x;
        changes.y = new_position.y;
        XConfigureWindow(m_windowing_engine->platform->display, m_window, CWX | CWY, &changes);
    }

    Window x11::window_x11::platform_handle() const {
        return m_window;
    }

    bool x11::window_x11::decorated() const {
        return m_decorated;
    }

    void x11::window_x11::decorated(bool new_mode) {
        if (new_mode ^ m_decorated) {
            // Change decoration

            m_decorated = new_mode;
            if (m_decorated) {
                PropMwmHints hints;
                Atom property;
                hints.flags = MWM_HINTS_DECORATIONS;
                hints.decorations = MWM_DECOR_ALL;
                property = XInternAtom(m_windowing_engine->platform->display, _XA_MWM_HINTS, true);
                XChangeProperty(m_windowing_engine->platform->display, m_window, property,
                                property, 32, PropModeReplace, (unsigned char *)&hints,
                                PROP_MWM_HINTS_ELEMENTS);
                SPDLOG_INFO("DECORATE");
            } else {
                PropMwmHints hints;
                Atom property;
                hints.flags = MWM_HINTS_DECORATIONS;
                hints.decorations = 0;
                property = XInternAtom(m_windowing_engine->platform->display, _XA_MWM_HINTS, true);
                XChangeProperty(m_windowing_engine->platform->display, m_window, property,
                                property, 32, PropModeReplace, (unsigned char *)&hints,
                                PROP_MWM_HINTS_ELEMENTS);
                SPDLOG_INFO("UNDECORATE");
            }
        }


    }

    void x11::window_x11::restore() {

    }

    void x11::window_x11::maximize() {

    }

    void x11::window_x11::minimize() {

    }
}
#endif