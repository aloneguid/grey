#pragma once

#include "roboto.inl"

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

namespace grey {
    void load_font(float scale, bool load_fa = true) {
        ImGuiIO io = ImGui::GetIO();

        ImFont* f = io.Fonts->AddFontFromMemoryCompressedTTF(
            Roboto_compressed_data, Roboto_compressed_size,
            16.0f * scale);

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
                float icon_font_size = 16.0f * scale;
                ImFontConfig config;
                config.OversampleH = 1;
                config.MergeMode = true;
                config.PixelSnapH = true;
                config.GlyphOffset.y = 3.0f * scale;
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
    }
}