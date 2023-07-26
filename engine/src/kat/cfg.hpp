#pragma once

#define KATWINDOW_X11 0
#define KATWINDOW_WIN32 1

#ifndef KATWINDOW_TARGET
#if defined(WIN32) || defined(WIN32_) || defined(__WIN32__) || defined(__NT__)
#define KATWINDOW_TARGET KATWINDOW_WIN32
#elif __APPLE__
#error "Apple operating systems are currently unsupported"
#elif __ANDROID__
#error "Android is unsupported"
#elif __linux__
#if __has_include("X11/Xlib.h")
#define KATWINDOW_TARGET KATWINDOW_X11
#else
#error "Determined linux environment but cannot find development headers for a supported windowing system. Check build environment (might need to install dev libraries for your windowing system library)"
#endif
#else
#error "Couldn't detect a valid system to build for. Try setting the KATWINDOW_TARGET preprocessor macro manually."
#endif
#endif

#if KATWINDOW_TARGET == KATWINDOW_X11
#define KATWINDOW_TARGET_X11
#elif KATWINDOW_TARGET == KATWINDOW_WIN32
#define KATWINDOW_TARGET_WIN32
#else
#error "Invalid value for KATWINDOW_TARGET preprocessor macro."
#endif

#ifndef KAT_BASE_DPI
#define KAT_BASE_DPI 96.f
#endif