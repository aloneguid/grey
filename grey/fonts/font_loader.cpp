#include "font_loader.h"
#include "MaterialIcons-Regular.inl"
#include "MaterialIcons.h"
#include <filesystem>
#include "imgui.h"
#include "../common/os.h"

#if PLATFORM_WINDOWS
// use built-in system fonts
#else
// on *nix use fontconfig to discover fonts, rather than embedding them inline
#include <fontconfig/fontconfig.h>

std::string GetDefaultFontPath(const char* family = "sans-serif") {
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pattern = FcNameParse(reinterpret_cast<const FcChar8*>(family));
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern* match = FcFontMatch(config, pattern, &result);

    std::string path;
    if(match) {
        FcChar8* file = nullptr;
        if(FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch)
            path = reinterpret_cast<const char*>(file);
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pattern);
    FcConfigDestroy(config);
    return path;
}

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
#else
        string path = GetDefaultFontPath();
        ImFont* font_system = io.Fonts->AddFontFromFileTTF(path.c_str(), default_font_size);

        if(cfg.load_fixed) {
            string path = GetDefaultFontPath("nonospace");
            font_fixed = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f);
        }
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
