cmake_minimum_required(VERSION 3.24)
project(katengine VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(glm CONFIG REQUIRED)

add_library(katengine src/kat/core/core.cpp src/kat/core/core.hpp src/kat/window/window.cpp src/kat/window/window.hpp src/kat/engine.hpp src/kat/window/x11/platform_x11.cpp src/kat/window/x11/platform_x11.hpp src/kat/cfg.hpp src/kat/window/utils.cpp src/kat/window/utils.hpp
        src/kat/window/win32/platform_win32.cpp
        src/kat/window/win32/platform_win32.hpp)
target_include_directories(katengine PUBLIC src/)

if (WIN32)
        set(KAT_PLATFORM_LIBS user32 kernel32 dwmapi shcore)
elseif(UNIX AND NOT APPLE)
        set(KAT_PLATFORM_LIBS Xm X11 Xrandr Xt)
endif()


target_link_libraries(katengine PUBLIC glm::glm spdlog::spdlog ${KAT_PLATFORM_LIBS})
target_compile_definitions(katengine PUBLIC
        $<$<CONFIG:Debug>:KAT_DEBUG> $<$<CONFIG:RelWithDebugInfo>:KAT_DEBUG>
        $<$<CONFIG:Release>:KAT_RELEASE> $<$<CONFIG:MinSizeRel>:KAT_RELEASE>
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=1> $<$<CONFIG:RelWithDebugInfo>:SPDLOG_ACTIVE_LEVEL=1>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=2> $<$<CONFIG:MinSizeRel>:SPDLOG_ACTIVE_LEVEL=2>
        $<$<CONFIG:Release>:SPDLOG_NO_SOURCE_LOC> $<$<CONFIG:MinSizeRel>:SPDLOG_NO_SOURCE_LOC>
        $<$<CONFIG:Debug>:KAT_PLATFORM_VERIFYINTERFACES>
        )

add_library(katengine::katengine ALIAS katengine)
