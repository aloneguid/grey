#include "font_loader.h"

//#include "font_awesome_6_regular_400.inl"
//#include "font_awesome_6.h"
//
//#include "font_awesome_6_brands_400.inl"
//#include "font_awesome_6_brands.h"

//#include "forkawesome.inl"
//#include "forkawesome.h"

#include "MaterialIcons-Regular.inl"
#include "MaterialIcons.h"

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

    ImFont* font_loader::fixed_size_font{nullptr};

    ImFont* font_loader::load_system_font(ImGuiIO& io) {
        ImFont* f{nullptr};
#if PLATFORM_WINDOWS
        string path = grey::common::os::get_system_fonts_path();
        // Segoe UI is the default UI font for Windows 10 and 11.
        path += "\\segoeui.ttf";
        f = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f);
#else
        string path = GetDefaultFontPath();
        f = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f);
#endif
        return f;

    }

    ImFont* font_loader::load_fixed_font(ImGuiIO& io) {
        ImFont* f{nullptr};
#if PLATFORM_WINDOWS
        string path = grey::common::os::get_system_fonts_path();
        // Segoe UI is the default UI font for Windows 10 and 11.
        path += "\\consola.ttf";
        f = io.Fonts->AddFontFromFileTTF(path.c_str(), 15.0f);
#else
        string path = GetDefaultFontPath("nonospace");
        f = io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0f);
#endif
        return f;
    }

    void font_loader::load_font(bool load_fa, bool load_fixed) {
        ImGuiIO& io = ImGui::GetIO();

        ImFont* f = load_system_font(io);

        if(f && load_fa) {

            // note that FA are very RAM heavy, it adds more than 100mb!!!

            // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
            //float icon_font_size = 16.0f * 2.0f / 3.0f * scale;

            // forkawesome
            /*{
                ImFontConfig config;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
                static const ImWchar icon_ranges[] = {ICON_MIN_FK, ICON_MAX_16_FK, 0};
                io.Fonts->AddFontFromMemoryCompressedTTF(
                    forkawesome_compressed_data, forkawesome_compressed_size,
                    icon_font_size,
                    &config, icon_ranges);
            }*/

            // Google Material Icons
            {
                float icon_font_size = 16.0f;
                ImFontConfig config;
                config.OversampleH = 1;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphOffset.y = 3.0f;
                config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
                static const ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_16_MD, 0};
                io.Fonts->AddFontFromMemoryCompressedTTF(
                    MaterialIconsRegular_compressed_data, MaterialIconsRegular_compressed_size,
                    icon_font_size,
                    &config, icon_ranges);
            }

            /*
            // fontawesome v6
            {
                ImFontConfig config;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
                static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
                io.Fonts->AddFontFromMemoryCompressedTTF(
                    font_awesome_regular_400_compressed_data, font_awesome_regular_400_compressed_size,
                    icon_font_size,
                    &config, icon_ranges);
            }

            // fontawesome v6 (brands)
            {
                ImFontConfig config;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
                static const ImWchar icon_ranges[] = {ICON_MIN_FAB, ICON_MAX_16_FAB, 0};
                io.Fonts->AddFontFromMemoryCompressedTTF(
                    font_awesome_6_brands_400_compressed_data,
                    font_awesome_6_brands_400_compressed_size,
                    icon_font_size,
                    &config, icon_ranges);
            }*/

        }

        if(load_fixed) {
            fixed_size_font = load_fixed_font(io);
        }
    }

    ImFont* font_loader::get_fixed_size_font(float scale) {
        return fixed_size_font;
    }
}