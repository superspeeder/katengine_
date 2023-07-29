cmake_minimum_required(VERSION 3.24)
project(katengine VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(glm CONFIG REQUIRED)

add_library(katengine src/kat/core/core.cpp src/kat/core/core.hpp src/kat/window/window.cpp src/kat/window/window.hpp src/kat/engine.hpp src/kat/window/x11/platform_x11.cpp src/kat/window/x11/platform_x11.hpp src/kat/cfg.hpp src/kat/window/utils.cpp src/kat/window/utils.hpp)
target_include_directories(katengine PUBLIC src/)
target_link_libraries(katengine PUBLIC glm::glm spdlog::spdlog Xm X11 GL Xrandr Xt)
target_compile_definitions(katengine PUBLIC
        $<$<CONFIG:Debug>:KAT_DEBUG> $<$<CONFIG:RelWithDebugInfo>:KAT_DEBUG>
        $<$<CONFIG:Release>:KAT_RELEASE> $<$<CONFIG:MinSizeRel>:KAT_RELEASE>
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=1> $<$<CONFIG:RelWithDebugInfo>:SPDLOG_ACTIVE_LEVEL=1>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=2> $<$<CONFIG:MinSizeRel>:SPDLOG_ACTIVE_LEVEL=2>
        $<$<CONFIG:Release>:SPDLOG_NO_SOURCE_LOC> $<$<CONFIG:MinSizeRel>:SPDLOG_NO_SOURCE_LOC>
        $<$<CONFIG:Debug>:KAT_PLATFORM_VERIFYINTERFACES>
        )

add_library(katengine::katengine ALIAS katengine)
