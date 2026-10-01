#include "model.h"
#include <format>

using namespace std;

namespace grey {
    rgb_colour::rgb_colour(const std::string& hex) {
        std::string h = hex;

        // Remove # prefix if present
        if(!h.empty() && h[0] == '#') {
            h = h.substr(1);
        }

        r = g = b = 0.0f;
        o = 1.0f;

        // Parse hex string (expects RRGGBB or RRGGBBAA format)
        if(h.length() >= 6) {
            try {
                unsigned long value = std::stoul(h.substr(0, 6), nullptr, 16);
                r = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
                g = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
                b = static_cast<float>(value & 0xFF) / 255.0f;
            } catch(...) {
            }
        }

        // parse opacity if present
        if(h.length() >= 8) {
            try {
                unsigned long value = std::stoul(h.substr(6, 2), nullptr, 16);
                o = static_cast<float>(value) / 255.0f;
            } catch(...) {
            }
        }
    }

    const std::string rgb_colour::to_hex(bool prepend_hash) const {
        std::string hex = std::format("{}{:02X}{:02X}{:02X}",
            prepend_hash ? "#" : "",
            static_cast<int>(r * 255),
            static_cast<int>(g * 255),
            static_cast<int>(b * 255));
        if(o < 1.0f) {
            hex += std::format("{:02X}", static_cast<int>(o * 255));
        }
        return hex;
    }

}