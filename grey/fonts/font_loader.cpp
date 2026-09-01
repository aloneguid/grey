#include "font_loader.h"
#include "MaterialIcons-Regular.inl"
#include "MaterialIcons.h"
#include <filesystem>
#include "imgui.h"
#include "../common/os.h"

#if PLATFORM_WINDOWS
// use built-in system fonts
#elif PLATFORM_LINUX
// on *nix use fontconfig to discover fonts, rather than embedding them inline
#include <fontconfig/fontconfig.h>

/**
 * @brief Font locator class using Fontconfig library to discover fonts. Wrapped in RAII style to prevent re-initalisation of fontconfig for each font lookup.
 */
class font_locator {
public:
    font_locator() {
        // Initializes the Fontconfig library
        FcInit();
        // Loads the config and builds the font cache (the expensive part)
        config_ = FcInitLoadConfigAndFonts();
    }

    ~font_locator() {
        if (config_) {
            FcConfigDestroy(config_);
        }
        // Cleans up Fontconfig global state
        FcFini();
    }

    // Delete copy constructor and assignment operator to prevent double-freeing config_
    font_locator(const font_locator&) = delete;
    font_locator& operator=(const font_locator&) = delete;

    // Allow moving
    font_locator(font_locator&& other) noexcept : config_(other.config_) {
        other.config_ = nullptr;
    }
    font_locator& operator=(font_locator&& other) noexcept {
        if (this != &other) {
            if (config_) FcConfigDestroy(config_);
            config_ = other.config_;
            other.config_ = nullptr;
        }
        return *this;
    }

    std::string get_default_font_path(const char* family = "sans-serif") const {
        if (!config_) return "";

        FcPattern* pattern = FcNameParse(reinterpret_cast<const FcChar8*>(family));
        if (!pattern) return "";

        FcConfigSubstitute(config_, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        FcResult result;
        FcPattern* match = FcFontMatch(config_, pattern, &result);

        std::string path;
        if (match) {
            FcChar8* file = nullptr;
            if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
                if (file) {
                    path = reinterpret_cast<const char*>(file);
                }
            }
            FcPatternDestroy(match);
        }

        FcPatternDestroy(pattern);
        return path;
    }

private:
    FcConfig* config_ = nullptr;
};

#endif

using namespace std;

namespace grey::fonts {

    ImFont* font_loader::font_fixed{nullptr};
    ImFont* font_loader::font_bold{nullptr};

    void font_loader::preload_fonts(const font_config& cfg) {
        ImGuiIO& io = ImGui::GetIO();
        constexpr float default_font_size = 18.0f;

#if PLATFORM_WINDOWS
        // always load default system font
        string fonts_path = grey::common::os::get_system_fonts_path();
        // Segoe UI is the default UI font for Windows 10 and 11.
        string default_font_path = fonts_path + "\\segoeui.ttf";
        ImFont* font_system = io.Fonts->AddFontFromFileTTF(default_font_path.c_str(), default_font_size);
        if(cfg.load_bold) {
            string bold_font_path = fonts_path + "\\segoeuib.ttf";
            font_bold = io.Fonts->AddFontFromFileTTF(bold_font_path.c_str(), default_font_size);
        }
        if(cfg.load_fixed) {
            // prefer Cascadia Code on Windows, but fall-back to Consolas
            string fixed_font_path = fonts_path + "\\cascadiacode.ttf";
            if(!std::filesystem::exists(fixed_font_path))
                fixed_font_path = fonts_path + "\\consola.ttf";
            font_fixed = io.Fonts->AddFontFromFileTTF(fixed_font_path.c_str(), default_font_size);
        }
#elif PLATFORM_LINUX
        font_locator fl;

        string normal_font_path = fl.get_default_font_path("sans-serif");
        ImFont* font_system = io.Fonts->AddFontFromFileTTF(normal_font_path.c_str(), default_font_size);

        if(cfg.load_bold) {
            string bold_font_path = fl.get_default_font_path("sans-serif:bold");
            font_bold = io.Fonts->AddFontFromFileTTF(bold_font_path.c_str(), default_font_size);
        }

        if(cfg.load_fixed) {
            string monospace_font_path = fl.get_default_font_path("monospace");
            font_fixed = io.Fonts->AddFontFromFileTTF(monospace_font_path.c_str(), 18.0f);
        }
#elif PLATFORM_MACOS
        ImFont* font_system = io.Fonts->AddFontDefault();
#endif

        if(cfg.load_icons) {
            // Google Material Icons
            float icon_font_size = 16.0f;
            ImFontConfig config;
            config.OversampleH = 1;
            config.MergeMode = true;
            config.PixelSnapH = true;
            config.GlyphOffset.y = 3.0f;
            config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
            config.DstFont = font_system;
            static const ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_16_MD, 0};
            io.Fonts->AddFontFromMemoryCompressedTTF(
                MaterialIconsRegular_compressed_data, MaterialIconsRegular_compressed_size,
                icon_font_size,
                &config, icon_ranges);
        }
    }

    ImFont* font_loader::get_font(font_weight weight) {
        if(weight == font_weight::fixed_size)   return font_fixed;
        if(weight == font_weight::bold)         return font_bold;
        return nullptr;
    }
}
