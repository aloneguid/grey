#pragma once
#include <string>

namespace grey::common::lin::os {
    /**
     * @brief Gets location of the system fonts folder.
     * On Linux, this is a bit more complex, so we might return a common path or use fc-match.
     * @return 
     */
    std::string get_system_fonts_path();

    /**
     * @brief Gets the path to a specific system font using fontconfig (fc-match).
     * @param font_name Generic name like "sans-serif" or "monospace".
     * @return Path to the font file.
     */
    std::string get_system_font_path(const std::string& font_name);
}
