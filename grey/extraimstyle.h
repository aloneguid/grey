#pragma once
#include <imgui.h>

namespace grey {

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
}