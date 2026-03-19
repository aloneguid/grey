#include "os.h"
#include <cstdio>
#include <memory>
#include <array>
#include <algorithm>

namespace grey::common::lin::os {

    struct pclose_deleter {
        void operator()(FILE* f) const { pclose(f); }
    };

    std::string get_system_font_path(const std::string& font_name) {
        std::string command = "fc-match -f \"%{file}\" " + font_name;
        std::array<char, 512> buffer;
        std::string result;
        std::unique_ptr<FILE, pclose_deleter> pipe(popen(command.c_str(), "r"));
        if (!pipe) {
            return "";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        // remove trailing newline if any
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        return result;
    }

    std::string get_system_fonts_path() {
        // On Linux there isn't a single "fonts" folder like on Windows.
        // We can return a common one, or just empty if we use get_system_font_path instead.
        return "/usr/share/fonts";
    }
}
