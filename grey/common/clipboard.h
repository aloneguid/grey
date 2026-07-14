#pragma once
#include <string>

namespace grey::common::clipboard {
    void set_text(const std::string& text);

    std::string get_text();
}
