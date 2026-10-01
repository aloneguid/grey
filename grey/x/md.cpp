#include "md.h"
#include "../widgets.h"
#include "../themes.h"

#include <algorithm>
#include <charconv>
#include <string_view>

namespace grey::widgets::x {
    namespace {
        struct userdata {
            md* m;
            const markdown_config& config;
        };
    }

    md::md() {
        m_md.abi_version = 0;

        m_md.flags = MD_FLAG_TABLES | MD_FLAG_UNDERLINE | MD_FLAG_STRIKETHROUGH;

        m_md.enter_block = [](MD_BLOCKTYPE t, void* detail, void* u) {
            auto* ud = static_cast<userdata *>(u);
            return ud->m->any_block(t, detail, true, ud->config);
        };

        m_md.leave_block = [](MD_BLOCKTYPE t, void* d, void* u) {
            auto* ud = static_cast<userdata *>(u);
            return ud->m->any_block(t, d, false, ud->config);
        };

        m_md.enter_span = [](MD_SPANTYPE t, void* d, void* u) {
            auto ud = static_cast<userdata *>(u);
            return ud->m->span(t, d, true, ud->config);
        };

        m_md.leave_span = [](MD_SPANTYPE t, void* d, void* u) {
            auto ud = static_cast<userdata *>(u);
            return ud->m->span(t, d, false, ud->config);
        };

        m_md.text = [](MD_TEXTTYPE t, const MD_CHAR* text, MD_SIZE size, void* u) {
            return ((userdata *) u)->m->text(t, text, text + size);
        };

        m_md.debug_log = nullptr;

        m_md.syntax = nullptr;

        ////////////////////////////////////////////////////////////////////////////

    }


    void md::BLOCK_UL(const MD_BLOCK_UL_DETAIL* d, bool e) {
        if(e) {
            m_list_stack.push_back(list_info{0, d->mark, false});
        } else {
            m_list_stack.pop_back();
            if(m_list_stack.empty())ImGui::NewLine();
        }
    }

    void md::BLOCK_OL(const MD_BLOCK_OL_DETAIL* d, bool e) {
        if(e) {
            m_list_stack.push_back(list_info{d->start, d->mark_delimiter, true});
        } else {
            m_list_stack.pop_back();
            if(m_list_stack.empty())ImGui::NewLine();
        }
    }

    void md::BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool e) {
        if(e) {
            ImGui::NewLine();

            list_info& nfo = m_list_stack.back();
            if(nfo.is_ol) {
                ImGui::Text("%d%c", nfo.cur_ol++, nfo.delim);
                ImGui::SameLine();
            } else {
                if(nfo.delim == '*') {
                    float cx = ImGui::GetCursorPosX();
                    cx -= ImGui::GetStyle().FramePadding.x * 2;
                    ImGui::SetCursorPosX(cx);
                    ImGui::Bullet();
                } else {
                    ImGui::Text("%c", nfo.delim);
                    ImGui::SameLine();
                }
            }

            ImGui::Indent();
        } else {
            ImGui::Unindent();
        }
    }

    void md::BLOCK_HR(bool e) {
        if(!e) {
            ImGui::NewLine();
            ImGui::Separator();
        }
    }

    void md::BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e, const markdown_config& cfg) {
        if(e) {
            m_hlevel = d->level;
            ImGui::NewLine();
        } else {
            m_hlevel = 0;
        }

        set_font(e, cfg);

        if(!e) {
            if(d->level <= 2) {
                ImGui::NewLine();
                ImGui::Separator();
            }
        }
    }

    void md::BLOCK_DOC(bool) {
    }

    void md::BLOCK_QUOTE(bool e) {
        if(e) {
            m_quote_stack.push_back({ImGui::GetCursorScreenPos()});
            ImGui::Indent();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        } else {
            if(!m_quote_stack.empty()) {
                const ImVec2 start = m_quote_stack.back().start;
                m_quote_stack.pop_back();
                const ImVec2 end = ImGui::GetCursorScreenPos();
                const ImGuiStyle& style = ImGui::GetStyle();
                const float line_x = start.x + style.FramePadding.x;
                const float line_start = start.y + ImGui::GetTextLineHeight() + style.ItemSpacing.y;
                const float line_end = std::max(end.y, line_start + ImGui::GetTextLineHeight());
                const float line_width = 5.0f;
                ImGui::GetWindowDrawList()->AddLine(
                    {line_x, line_start}, {line_x, line_end}, ImGui::GetColorU32(ImGuiCol_Border), line_width);
            }
            ImGui::PopStyleColor();
            ImGui::Unindent();
            ImGui::NewLine();
        }
    }

    void md::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e) {
        if(e) {
            ImGui::NewLine();
        }

        m_is_code = e;
        set_font(e, {});

        if(!e) {
            ImGui::NewLine();
        }
    }

    void md::BLOCK_HTML(bool) {
    }

    void md::BLOCK_P(bool) {
        if(!m_list_stack.empty())return;
        ImGui::NewLine();
    }

    void md::BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL* d, bool e) {
        if(e) {
            m_table_context = true;
            m_table_active = false;

            if(d && d->col_count > 0) {
                // Keep table IDs allocation-free; tables can be opened frequently while rendering.
                char table_id[32] = "##markdown_table_";
                const auto table_id_end = std::to_chars(
                    table_id + sizeof("##markdown_table_") - 1, table_id + sizeof(table_id), m_table_index++);
                *table_id_end.ptr = '\0';
                ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
                if(m_table_border) flags |= ImGuiTableFlags_Borders;
                m_table_active = ImGui::BeginTable(table_id, d->col_count, flags);
            }

        } else {
            if(m_table_active) {
                ImGui::EndTable();
                ImGui::NewLine();
            }

            m_table_active = false;
            m_table_context = false;
            m_is_table_header = false;
            m_table_align = MD_ALIGN_DEFAULT;
        }
    }

    void md::BLOCK_THEAD(bool e, const markdown_config& cfg) {
        m_is_table_header = e;
        if(m_table_header_highlight)set_font(e, cfg);
    }

    void md::BLOCK_TBODY(bool) {
    }

    void md::BLOCK_TR(bool e) {
        if(e && m_table_active) ImGui::TableNextRow();
    }

    void md::BLOCK_TH(const MD_BLOCK_TD_DETAIL* d, bool e) {
        BLOCK_TD(d, e);
    }

    void md::BLOCK_TD(const MD_BLOCK_TD_DETAIL* d, bool e) {
        if(e) {
            m_table_align = d ? d->align : MD_ALIGN_DEFAULT;
            if(m_table_active) ImGui::TableNextColumn();
        } else {
            m_table_align = MD_ALIGN_DEFAULT;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    void md::set_href(bool e, const MD_ATTRIBUTE& src) {
        if(e) {
            m_href.assign(src.text, src.size);
        } else {
            m_href.clear();
        }
    }

    void md::set_font(bool e, const markdown_config& config) {
        if(e) {
            ImFont* font{nullptr};
            float font_size;
            make_font(&font, font_size, config);
            ImGui::PushFont(font, font_size);
        } else {
            ImGui::PopFont();
        }
    }

    void md::set_color(bool e) {
        if(e) {
            ImGui::PushStyleColor(ImGuiCol_Text, get_color());
        } else {
            ImGui::PopStyleColor();
        }
    }

    void md::line(ImColor c, bool under) {
        ImVec2 mi = ImGui::GetItemRectMin();
        ImVec2 ma = ImGui::GetItemRectMax();

        if(!under) {
            ma.y -= ImGui::GetFontSize() / 2;
        }

        mi.y = ma.y;

        ImGui::GetWindowDrawList()->AddLine(mi, ma, c, 1.0f);
    }

    void md::SPAN_A(const MD_SPAN_A_DETAIL* d, bool e, const markdown_config& cfg) {
        set_href(e, d->href);
        set_color(e);
    }


    void md::SPAN_EM(bool e, const markdown_config& cfg) {
        m_is_em = e;
        set_font(e, cfg);
    }

    void md::SPAN_STRONG(bool e, const markdown_config& cfg) {
        m_is_strong = e;
        set_font(e, cfg);
    }


    void md::SPAN_IMG(const MD_SPAN_IMG_DETAIL* d, bool e) {
        m_is_image = e;

        set_href(e, d->src);

        if(e) {
            image_info nfo;
            if(get_image(nfo)) {
                const float scale = ImGui::GetIO().FontGlobalScale;
                nfo.size.x *= scale;
                nfo.size.y *= scale;


                ImVec2 const csz = ImGui::GetContentRegionAvail();
                if(nfo.size.x > csz.x) {
                    const float r = nfo.size.y / nfo.size.x;
                    nfo.size.x = csz.x;
                    nfo.size.y = csz.x * r;
                }

                ImGui::Image(nfo.texture_id, nfo.size, nfo.uv0, nfo.uv1, nfo.col_tint, nfo.col_border);

                if(ImGui::IsItemHovered()) {
                    //if (d->title.size) {
                    //	ImGui::SetTooltip("%.*s", (int)d->title.size, d->title.text);
                    //}

                    if(ImGui::IsMouseReleased(0)) {
                        open_url();
                    }
                }
            }
        }
    }

    void md::render_inline_code(bool enter) {
        m_is_inline_code = enter;
        set_font(enter, {});
    }

    void md::SPAN_LATEXMATH(bool) {
    }

    void md::SPAN_LATEXMATH_DISPLAY(bool) {
    }

    void md::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {
    }

    void md::SPAN_U(bool e) {
        m_is_underline = e;
    }

    void md::SPAN_DEL(bool e) {
        m_is_strikethrough = e;
    }

    bool md::render_alert(const char* str, const char* str_end) {
        if(m_quote_stack.empty() || !m_quote_stack.back().alert_candidate) return false;

        const char* marker = str;
        while(marker < str_end && (*marker == ' ' || *marker == '\t')) ++marker;

        struct alert_info {
            std::string_view marker;
            const char* icon;
            const char* label;
            emphasis color;
        };

        static constexpr alert_info alerts[] = {
            {"[!NOTE]", ICON_MD_INFO, "Note", emphasis::info},
            {"[!TIP]", ICON_MD_LIGHTBULB, "Tip", emphasis::success},
            {"[!IMPORTANT]", ICON_MD_FEEDBACK, "Important", emphasis::primary},
            {"[!WARNING]", ICON_MD_WARNING, "Warning", emphasis::warning},
            {"[!CAUTION]", ICON_MD_REPORT, "Caution", emphasis::error},
        };

        const alert_info* alert = nullptr;
        for(const alert_info& candidate : alerts) {
            // string_view avoids repeated strlen calls while checking alert markers.
            const size_t marker_size = candidate.marker.size();
            if(static_cast<size_t>(str_end - marker) >= marker_size
                && std::string_view(marker, marker_size) == candidate.marker) {
                if(marker + marker_size == str_end || marker[marker_size] == ' ' || marker[marker_size] == '\t'
                    || marker[marker_size] == '\r' || marker[marker_size] == '\n') {
                    alert = &candidate;
                    break;
                }
            }
        }

        m_quote_stack.back().alert_candidate = false;
        if(!alert) return false;

        const ImGuiStyle& style = ImGui::GetStyle();
        ImFont* font = nullptr;
        float font_size = style.FontSizeBase;
        texter::make_font(0, font_weight::bold, &font, font_size);
        ImGuiCol color = ImGuiCol_Text;
        switch(alert->color) {
            case emphasis::primary:
                ImGui::PushStyleColor(ImGuiCol_Text, grey::themes::GreyColors[grey::themes::GreyCol_EmphasisPrimary]);
                break;
            case emphasis::success:
                ImGui::PushStyleColor(ImGuiCol_Text, grey::themes::GreyColors[grey::themes::GreyCol_EmphasisSuccess]);
                break;
            case emphasis::error:
                ImGui::PushStyleColor(ImGuiCol_Text, grey::themes::GreyColors[grey::themes::GreyCol_EmphasisError]);
                break;
            case emphasis::warning:
                ImGui::PushStyleColor(ImGuiCol_Text, grey::themes::GreyColors[grey::themes::GreyCol_EmphasisWarning]);
                break;
            case emphasis::info:
                ImGui::PushStyleColor(ImGuiCol_Text, grey::themes::GreyColors[grey::themes::GreyCol_EmphasisInfo]);
                break;
            default:
                ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[color]);
                break;
        }
        ImGui::TextUnformatted(alert->icon);
        sl();
        ImGui::PushFont(font, font_size);
        ImGui::TextUnformatted(alert->label);
        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::NewLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y * 6.0f);

        marker += alert->marker.size();
        while(marker < str_end && (*marker == ' ' || *marker == '\t')) ++marker;
        if(marker < str_end) render_text(marker, str_end);
        return true;
    }

    void md::render_text(const char* str, const char* str_end) const {
        const float scale = ImGui::GetIO().FontGlobalScale;
        const ImGuiStyle& s = ImGui::GetStyle();
        ImFont* font = ImGui::GetFont();
        const bool no_wrap = m_is_code || m_is_inline_code || m_is_table_header || m_table_active;
        const bool centered = m_table_active
            && (m_table_align == MD_ALIGN_CENTER || m_table_align == MD_ALIGN_RIGHT);
        const bool has_href = !m_href.empty();
        const bool underline = m_is_underline;
        const bool strikethrough = m_is_strikethrough;
        bool is_lf = false;

        // Cache span-invariant state so the hot rendering loop does not repeat member checks and lookups.
        if(!m_is_image) while(str < str_end) {
            const char* te = str_end;
            float x_left = avail_x();

            if(!no_wrap) {
                const float wrap_width = x_left * 0.5f;

                te = font->CalcWordWrapPositionA(scale, str, str_end, wrap_width);

                if(te == str) ++te;
            }

            if(centered) {
                const float text_width = ImGui::CalcTextSize(str, te).x;
                float offset = 0.0f;

                if(x_left > text_width) {
                    offset = m_table_align == MD_ALIGN_CENTER
                        ? (x_left - text_width) / 2.0f
                        : x_left - text_width;
                }

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            }

            ImGui::TextUnformatted(str, te);

            if(te > str && *(te - 1) == '\n') {
                is_lf = true;
            }

            if(has_href) {
                ImColor c;
                if(ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", m_href.c_str());

                    c = s.Colors[ImGuiCol_ButtonHovered];
                    if(ImGui::IsMouseReleased(0)) {
                        open_url();
                    }
                } else {
                    c = s.Colors[ImGuiCol_Button];
                }
                line(c, true);
            }

            if(underline) {
                line(s.Colors[ImGuiCol_Text], true);
            }

            if(strikethrough) {
                line(s.Colors[ImGuiCol_Text], false);
            }

            str = te;

            if(!m_is_code && !m_is_inline_code) {
                while(str < str_end && *str == ' ')++str;
            }
        }

        if(!is_lf)ImGui::SameLine(0.0f, 0.0f);
    }


    bool md::render_entity(const char* str, const char* str_end) {
        // Compare the bounded parser span without strlen or a temporary string.
        if(std::string_view(str, str_end - str) == "&nbsp;") {
            ImGui::TextUnformatted("");
            ImGui::SameLine();
            return true;
        }
        return false;
    }

    static std::string_view get_div_class(const char* str, const char* str_end) {
        // Keep HTML attribute parsing non-owning; this span is only needed during the callback.
        std::string_view div(str, str_end - str);
        if(!div.empty() && div.back() == '>') div.remove_suffix(1);

        size_t p = div.find("class");
        if(p == std::string_view::npos) return {};
        p += sizeof("class") - 1;

        const auto skip_spaces = [&div](size_t& pos) {
            while(pos < div.size() && (div[pos] == ' ' || div[pos] == '\t')) ++pos;
            return pos < div.size();
        };

        if(!skip_spaces(p) || div[p] != '=') return {};
        ++p;
        if(!skip_spaces(p)) return {};

        const bool quoted = div[p] == '"' || div[p] == '\'';
        const char quote = quoted ? div[p++] : '\0';
        const size_t end = quoted
            ? div.find(quote, p)
            : div.find_first_of(" \t", p);
        return div.substr(p, end == std::string_view::npos ? div.size() - p : end - p);
    }

    bool md::check_html(const char* str, const char* str_end) {
        // HTML callbacks provide bounded spans, so view comparisons avoid repeated strlen/strncmp work.
        const std::string_view html(str, str_end - str);

        if(html == "<br>") {
            ImGui::NewLine();
            return true;
        }
        if(html == "<hr>") {
            ImGui::Separator();
            return true;
        }
        if(html == "<u>") {
            m_is_underline = true;
            return true;
        }
        if(html == "</u>") {
            m_is_underline = false;
            return true;
        }

        constexpr std::string_view div_tag = "<div";
        if(html.size() >= div_tag.size() && html.starts_with(div_tag)) {
            m_div_stack.emplace_back(get_div_class(str + div_tag.size(), str_end));
            html_div(m_div_stack.back(), true);
            return true;
        }
        if(html == "</div>") {
            if(m_div_stack.empty())return false;
            html_div(m_div_stack.back(), false);
            m_div_stack.pop_back();
            return true;
        }
        return false;
    }


    void md::html_div(const std::string& dclass, bool e) {
        //Example:
#if 0
        if(dclass == "red") {
            if(e) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            } else {
                ImGui::PopStyleColor();
            }
        }
#endif
        dclass;
        e;
    }


    int md::text(MD_TEXTTYPE type, const char* str, const char* str_end) {
        if(m_table_context && !m_table_active) return 0;

        switch(type) {
            case MD_TEXT_NORMAL:
                if(render_alert(str, str_end)) break;
                render_text(str, str_end);
                break;
            case MD_TEXT_CODE:
                render_text(str, str_end);
                break;
            case MD_TEXT_NULLCHAR:
                break;
            case MD_TEXT_BR:
                ImGui::NewLine();
                break;
            case MD_TEXT_SOFTBR:
                soft_break();
                break;
            case MD_TEXT_ENTITY:
                if(!render_entity(str, str_end)) {
                    render_text(str, str_end);
                };
                break;
            case MD_TEXT_HTML:
                if(!check_html(str, str_end)) {
                    render_text(str, str_end);
                }
                break;
            case MD_TEXT_LATEXMATH:
                render_text(str, str_end);
                break;
            default:
                break;
        }

        return 0;
    }

    int md::any_block(MD_BLOCKTYPE type, void* d, bool enter, const markdown_config& cfg) {
        // label(format("{} {}", enter ? "enter" : "leave", magic_enum::enum_name(type)));

        if(type != MD_BLOCK_TABLE && m_table_context && !m_table_active) return 0;

        switch(type) {
            case MD_BLOCK_DOC:
                BLOCK_DOC(enter);
                break;
            case MD_BLOCK_QUOTE:
                BLOCK_QUOTE(enter);
                break;
            case MD_BLOCK_UL:
                BLOCK_UL((MD_BLOCK_UL_DETAIL *) d, enter);
                break;
            case MD_BLOCK_OL:
                BLOCK_OL((MD_BLOCK_OL_DETAIL *) d, enter);
                break;
            case MD_BLOCK_LI:
                BLOCK_LI((MD_BLOCK_LI_DETAIL *) d, enter);
                break;
            case MD_BLOCK_HR:
                BLOCK_HR(enter);
                break;
            case MD_BLOCK_H:
                BLOCK_H((MD_BLOCK_H_DETAIL *) d, enter, cfg);
                break;
            case MD_BLOCK_CODE:
                BLOCK_CODE((MD_BLOCK_CODE_DETAIL *) d, enter);
                break;
            case MD_BLOCK_HTML:
                BLOCK_HTML(enter);
                break;
            case MD_BLOCK_P:
                BLOCK_P(enter);
                break;
            case MD_BLOCK_TABLE:
                BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL *) d, enter);
                break;
            case MD_BLOCK_THEAD:
                BLOCK_THEAD(enter, cfg);
                break;
            case MD_BLOCK_TBODY:
                BLOCK_TBODY(enter);
                break;
            case MD_BLOCK_TR:
                BLOCK_TR(enter);
                break;
            case MD_BLOCK_TH:
                BLOCK_TH((MD_BLOCK_TD_DETAIL *) d, enter);
                break;
            case MD_BLOCK_TD:
                BLOCK_TD((MD_BLOCK_TD_DETAIL *) d, enter);
                break;
            default:
                assert(false);
                break;
        }

        return 0;
    }

    int md::span(MD_SPANTYPE type, void* d, bool e, const markdown_config& cfg) {
        if(m_table_context && !m_table_active) return 0;

        switch(type) {
            case MD_SPAN_EM:
                SPAN_EM(e, cfg);
                break;
            case MD_SPAN_STRONG:
                SPAN_STRONG(e, cfg);
                break;
            case MD_SPAN_A:
                SPAN_A((MD_SPAN_A_DETAIL *) d, e, cfg);
                break;
            case MD_SPAN_IMG:
                SPAN_IMG((MD_SPAN_IMG_DETAIL *) d, e);
                break;
            case MD_SPAN_CODE:
                render_inline_code(e);
                break;
            case MD_SPAN_DEL:
                SPAN_DEL(e);
                break;
            case MD_SPAN_LATEXMATH:
                SPAN_LATEXMATH(e);
                break;
            case MD_SPAN_LATEXMATH_DISPLAY:
                SPAN_LATEXMATH_DISPLAY(e);
                break;
            case MD_SPAN_WIKILINK:
                SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL *) d, e);
                break;
            case MD_SPAN_U:
                SPAN_U(e);
                break;
            default:
                assert(false);
                break;
        }

        return 0;
    }

    int md::print(const char* str, const char* str_end, const markdown_config& config) {
        m_is_code = false;
        m_is_inline_code = false;
        m_quote_stack.clear();
        if(str >= str_end) return 0;
        m_table_index = 0;
        m_table_context = false;
        m_table_active = false;
        m_table_align = MD_ALIGN_DEFAULT;
        userdata ud = {this, config};
        const int result = md_parse(str, (MD_SIZE) (str_end - str), &m_md, &ud);
        m_quote_stack.clear();
        return result;
    }

    bool md::make_font(ImFont** font, float& font_size, const markdown_config& config) const {
        *font = nullptr;
        font_size = ImGui::GetStyle().FontSizeBase;

        if(m_is_code || m_is_inline_code) {
            texter::make_font(0, font_weight::fixed_size, font, font_size);
            if(*font) {
                font_size = (*font)->LegacySize;
            }
            return true;
        }

        if(m_is_table_header) {
            texter::make_font(0, font_weight::bold, font, font_size);
            return true;
        }

        switch(m_hlevel) {
            case 0:
                if(m_is_strong) {
                    texter::make_font(0, font_weight::bold, font, font_size);
                    return true;
                }
                break;

            case 1:
                texter::make_font(config.h1_size_delta, font_weight::regular, font, font_size);
                return true;

            case 2:
                texter::make_font(config.h2_size_delta, font_weight::regular, font, font_size);
                return true;

            case 3:
                texter::make_font(config.h3_size_delta, font_weight::regular, font, font_size);
                return true;
        }

        return false;
    };

    ImVec4 md::get_color() const {
        if(!m_href.empty()) {
            return ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
        }
        return ImGui::GetStyle().Colors[ImGuiCol_Text];
    }


    bool md::get_image(image_info& nfo) const {
        //Use m_href to identify images

        //Example - Imgui font texture
        // nfo.texture_id = ImGui::GetIO().Fonts->TexID;
        nfo.size = {100, 50};
        nfo.uv0 = {0, 0};
        nfo.uv1 = {1, 1};
        nfo.col_tint = {1, 1, 1, 1};
        nfo.col_border = {0, 0, 0, 0};

        return true;
    };

    void md::open_url() const {
        ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
        if(pio.Platform_OpenInShellFn) {
            pio.Platform_OpenInShellFn(ImGui::GetCurrentContext(), m_href.c_str());
        }
    }

    void md::soft_break() {
        //Example:
#if 0
        ImGui::NewLine();
#endif
    }
}
