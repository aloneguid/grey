#include "backend.h"
#include "backends/win32dx11.h"
#include "imgui.h"
#include "imconfig.h"

// fonts (some are disabled to minimize the output binary size)
// I found Roboto Mono really pleasant
//#include "fonts/Sweet16_ImGui.inl"
//#include "fonts/Sweet16mono_ImGui.inl"
#include "fonts/font_awesome_6.inl"
#include "fonts/font_awesome_6_brands.inl"
#include "fonts/font_awesome_6.h"
#include "fonts/font_awesome_6_brands.h"
#include "fonts/roboto.inl"
#include <iostream>
//#include "fonts/opensans.inl"

using namespace std;

namespace grey
{

    // From https://github.com/procedural/gpulib/blob/master/gpulib_imgui.h
    struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

    std::unique_ptr<grey::backend> backend::make_platform_default(const string& title) {
        auto be = make_unique<grey::backends::win32dx11>(title);
        //be->set_theme(colour_theme::corp_grey);

        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // disable built-in .ini file creation
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        // set default theme and font
        be->set_theme(colour_theme::follow_os);
        be->set_default_font(font::roboto);

        return be;
    }

    std::vector<app_theme> backend::list_themes() {
        return std::vector<app_theme>{
            { "follow_os", "Follow OS", colour_theme::follow_os },
            {"dark", "Dark", colour_theme::dark},
            {"light", "Light", colour_theme::light},
            {"light2", "Alternative Light", colour_theme::light2},
            {"classic", "Classic", colour_theme::classic},
            {"corp_grey", "Corporate Grey", colour_theme::corp_grey},
            {"cherry", "Cherry", colour_theme::cherry}
        };
    }

    void backend::set_theme(colour_theme theme) {
        this->theme = theme;

        // see more https://github.com/ocornut/imgui/issues/707

        float scale = get_system_scale();

        switch(theme) {
            case grey::colour_theme::dark:
                ImGui::StyleColorsDark();
                break;
            case grey::colour_theme::light:
                ImGui::StyleColorsLight();
                break;
            case grey::colour_theme::light2:
            {
                static ImVec3 color_for_text = ImVec3(236.f / 255.f, 240.f / 255.f, 241.f / 255.f);
                static ImVec3 color_for_head = ImVec3(41.f / 255.f, 128.f / 255.f, 185.f / 255.f);
                static ImVec3 color_for_area = ImVec3(57.f / 255.f, 79.f / 255.f, 105.f / 255.f);
                static ImVec3 color_for_body = ImVec3(44.f / 255.f, 62.f / 255.f, 80.f / 255.f);
                static ImVec3 color_for_pops = ImVec3(33.f / 255.f, 46.f / 255.f, 60.f / 255.f);

                ImGuiStyle& style = ImGui::GetStyle();

                style.Colors[ImGuiCol_Text] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
                style.Colors[ImGuiCol_TextDisabled] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
                style.Colors[ImGuiCol_WindowBg] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.95f);
                //style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.58f);
                style.Colors[ImGuiCol_Border] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
                style.Colors[ImGuiCol_BorderShadow] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
                style.Colors[ImGuiCol_FrameBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
                style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
                style.Colors[ImGuiCol_FrameBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_TitleBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
                style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
                style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
                style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
                style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
                style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
                style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                //style.Colors[ImGuiCol_ComboBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
                style.Colors[ImGuiCol_CheckMark] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
                style.Colors[ImGuiCol_SliderGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
                style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_Button] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
                style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
                style.Colors[ImGuiCol_ButtonActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_Header] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
                style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
                style.Colors[ImGuiCol_HeaderActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                //style.Colors[ImGuiCol_Column] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.32f);
                //style.Colors[ImGuiCol_ColumnHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
                //style.Colors[ImGuiCol_ColumnActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_ResizeGrip] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.15f);
                style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
                style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                //style.Colors[ImGuiCol_CloseButton] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.16f);
                //style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.39f);
                //style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
                style.Colors[ImGuiCol_PlotLines] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
                style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_PlotHistogram] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
                style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
                style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
                style.Colors[ImGuiCol_PopupBg] = ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
                //style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.73f);

            }
            break;
            case grey::colour_theme::classic:
                ImGui::StyleColorsClassic();
                break;
            case grey::colour_theme::corp_grey:
            {
                ImGuiStyle* style = &ImGui::GetStyle();
                ImVec4* colors = style->Colors;

                colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
                colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
                colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
                colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
                colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
                colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
                colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
                colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
                colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
                colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
                colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
                colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
                colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
                colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
                colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
                colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
                colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
                colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
                colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
                colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
                colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
                colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
                colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
                colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
                colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
                colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
                colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
                colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
                colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
                //colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
                //colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
                colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
                colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
                colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
                colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
                colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
                colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
            }
            break;
            case colour_theme::cherry: {
                // cherry colors, 3 intensities
#define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
#define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
#define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
// backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.200f, 0.220f, 0.270f, v)
// text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

                auto& style = ImGui::GetStyle();
                style.Colors[ImGuiCol_Text] = TEXT(0.78f);
                style.Colors[ImGuiCol_TextDisabled] = TEXT(0.28f);
                style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
                //style.Colors[ImGuiCol_ChildWindowBg] = BG(0.58f);
                style.Colors[ImGuiCol_PopupBg] = BG(0.9f);
                style.Colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
                style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                style.Colors[ImGuiCol_FrameBg] = BG(1.00f);
                style.Colors[ImGuiCol_FrameBgHovered] = MED(0.78f);
                style.Colors[ImGuiCol_FrameBgActive] = MED(1.00f);
                style.Colors[ImGuiCol_TitleBg] = LOW(1.00f);
                style.Colors[ImGuiCol_TitleBgActive] = HI(1.00f);
                style.Colors[ImGuiCol_TitleBgCollapsed] = BG(0.75f);
                style.Colors[ImGuiCol_MenuBarBg] = BG(0.47f);
                style.Colors[ImGuiCol_ScrollbarBg] = BG(1.00f);
                style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
                style.Colors[ImGuiCol_ScrollbarGrabHovered] = MED(0.78f);
                style.Colors[ImGuiCol_ScrollbarGrabActive] = MED(1.00f);
                style.Colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
                style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
                style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
                style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
                style.Colors[ImGuiCol_ButtonHovered] = MED(0.86f);
                style.Colors[ImGuiCol_ButtonActive] = MED(1.00f);
                style.Colors[ImGuiCol_Header] = MED(0.76f);
                style.Colors[ImGuiCol_HeaderHovered] = MED(0.86f);
                style.Colors[ImGuiCol_HeaderActive] = HI(1.00f);
                //style.Colors[ImGuiCol_Column] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
                //style.Colors[ImGuiCol_ColumnHovered] = MED(0.78f);
                //style.Colors[ImGuiCol_ColumnActive] = MED(1.00f);
                style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
                style.Colors[ImGuiCol_ResizeGripHovered] = MED(0.78f);
                style.Colors[ImGuiCol_ResizeGripActive] = MED(1.00f);
                style.Colors[ImGuiCol_PlotLines] = TEXT(0.63f);
                style.Colors[ImGuiCol_PlotLinesHovered] = MED(1.00f);
                style.Colors[ImGuiCol_PlotHistogram] = TEXT(0.63f);
                style.Colors[ImGuiCol_PlotHistogramHovered] = MED(1.00f);
                style.Colors[ImGuiCol_TextSelectedBg] = MED(0.43f);
                // [...]
                //style.Colors[ImGuiCol_ModalWindowDarkening] = BG(0.73f);
                style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
            }
            break;

            default:
                break;
        }

        // my personal touch
        ImGuiStyle& style = ImGui::GetStyle();
        //style.TabBorderSize = 5;
        style.FrameRounding =
            style.WindowRounding =
            style.ChildRounding =
            style.TabRounding =
            3 * scale;
        //style.WindowRounding = 0;
        //style.FrameBorderSize = 1.0f;
    }

    void backend::set_theme(const std::string& theme_id) {
        for(auto& theme : list_themes()) {
            if(theme.id == theme_id) {
                set_theme(theme.theme);
                break;
            }
        }
    }

    void backend::set_default_font(font font) {
        ImGuiIO io = ImGui::GetIO();

        ImFont* f{nullptr};
        float scale = get_system_scale();

        switch(font) {
            case font::proggy_clean:
                f = io.Fonts->AddFontDefault();
                break;
                //case font::sweet_16_mono:
                //    f = AddSweet16MonoFont();
                //    break;
            case font::roboto:
                f = io.Fonts->AddFontFromMemoryCompressedTTF(
                    Roboto_compressed_data, Roboto_compressed_size,
                    16.0f * scale);
                break;
                //case font::opensans:
                //    f = io.Fonts->AddFontFromMemoryCompressedTTF(
                //        OpenSansRegular_compressed_data, OpenSansRegular_compressed_size,
                //        20.0f * scale);
                //    break;
        }

        if(f) {

            // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
            //float icon_font_size = 16.0f * 2.0f / 3.0f * scale;
            float icon_font_size = 16.0f * scale;

            // fontawesome v6
            {
                ImFontConfig config;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphMinAdvanceX = icon_font_size; // Use if you want to make the icon monospaced
                static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
                io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_6_compressed_data, font_awesome_6_compressed_size,
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
                    font_awesome_6_brands_compressed_data,
                    font_awesome_6_brands_compressed_size,
                    icon_font_size,
                    &config, icon_ranges);
            }

        }
    }

    void backend::enable_docking() {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void backend::attach(std::shared_ptr<grey::window> w) {
        windows_dirty = true;
        windows_new = windows;
        windows_new.push_back(w);
    }

    void backend::detach(std::string window_id) {
        windows_dirty = true;
        windows_new.clear();
        for(shared_ptr<window> w : windows) {
            if(w->id == window_id) {
#if _DEBUG
                cout << "window #" << window_id << " detached" << endl;
#endif
            } else {
                windows_new.push_back(w);
            }
        }
    }

    bool backend::any_window_visible() {
        for(auto w : windows) {
            if(w->is_visible) return true;
        }

        return false;
    }
}