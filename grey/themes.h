#pragma once
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"

namespace grey::themes {
    struct app_theme {
        std::string id;
        std::string name;
        bool is_dark;
        ImU32 accent;
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

    // theme change callback, called from the app so that the app can apply OS specific theme changes

    const std::string FollowOsThemeId = "follow_os";

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

    app_theme get_theme(const std::string& theme_id);

    /**
     * @brief Changes ImGui theme to the one specified by theme_id. You should prefer calling set_theme on the app instance instead.
     * @param theme_id 
     * @param scale 
     */
    void set_theme(const std::string& theme_id, float scale);

    bool is_dark_theme(const std::string& theme_id);
}