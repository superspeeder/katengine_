#include "utils.hpp"

glm::vec2 kat::window::conv_dpi_to_scale(const glm::vec2& dpi) noexcept {
    return dpi / static_cast<float>(KAT_BASE_DPI);
}

bool kat::window::video_mode::operator==(const kat::window::video_mode &rhs) const {
    return resolution == rhs.resolution &&
           refresh_rate == rhs.refresh_rate &&
           depth == rhs.depth;
}

bool kat::window::video_mode::operator!=(const kat::window::video_mode &rhs) const {
    return !(rhs == *this);
}

bool kat::window::display_depth::operator==(const kat::window::display_depth &rhs) const {
    return red == rhs.red &&
           green == rhs.green &&
           blue == rhs.blue;
}

bool kat::window::display_depth::operator!=(const kat::window::display_depth &rhs) const {
    return !(rhs == *this);
}
