#include "utils.hpp"

glm::vec2 kat::window::conv_dpi_to_scale(const glm::vec2& dpi) noexcept {
    return dpi / static_cast<float>(KAT_BASE_DPI);
}
