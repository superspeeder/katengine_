#pragma once

#include "kat/cfg.hpp"
#include <glm/glm.hpp>

namespace kat::window {
    glm::vec2 conv_dpi_to_scale(const glm::vec2& dpi) noexcept;

    struct display_depth {
        uint32_t red, green, blue;

        bool operator==(const display_depth &rhs) const;

        bool operator!=(const display_depth &rhs) const;
    };

    struct video_mode {
        glm::uvec2 resolution;
        int refresh_rate;
        display_depth depth;

        bool operator==(const video_mode &rhs) const;

        bool operator!=(const video_mode &rhs) const;
    };
}