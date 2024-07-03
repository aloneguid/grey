#pragma once
#include <string>
#include <vector>
#include "imgui.h"

namespace grey::themes {
    struct app_theme {
        const std::string id;
        const std::string name;
        const bool is_dark;
        const ImU32 accent;
    };

    enum GreyCol_ {
        GreyCol_EmphasisPrimary = 0,
        GreyCol_EmphasisPrimaryHovered,
        GreyCol_EmphasisPrimaryActive,
        GreyCol_EmphasisError,
        GreyCol_EmphasisErrorHovered,
        GreyCol_EmphasisErrorActive,
        GreyCol_Count
    };

    // extra theming constants
    extern ImVec4 GreyColors[GreyCol_Count];

    const std::string FollowOsThemeName = "follow_os";

    std::vector<app_theme> list_themes();

    void set_theme_dark();
    void set_theme_light();
    void set_theme_follow_os();
    void set_theme_cherry();
    void set_theme_enemymouse();
    void set_theme_gold();
    void set_theme_microsoft();
    void set_theme_darcula();
    void set_theme_steam();
    void set_theme_duck_red();

    void set_theme(const std::string& theme_id, float scale);
}