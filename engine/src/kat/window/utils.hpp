#pragma once

#include "kat/cfg.hpp"
#include <glm/glm.hpp>

namespace kat::window {
    glm::vec2 conv_dpi_to_scale(const glm::vec2& dpi) noexcept;
}