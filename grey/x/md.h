#pragma once

#include "md4c.h"
#include "imgui.h"
#include <string>
#include <vector>
#include "../widgets.h"

namespace grey::widgets::x {
    struct md {
        md();

        virtual ~md() = default;

        //returns 0 on success
        int print(const char* str, const char* str_end, const markdown_config& config);

        //for example, these flags can be changed in div callback

        //draw border
        bool m_table_border = true;
        //render header in a different way than other rows
        bool m_table_header_highlight = true;

    protected:
        virtual void BLOCK_DOC(bool);

        virtual void BLOCK_QUOTE(bool);

        virtual void BLOCK_UL(const MD_BLOCK_UL_DETAIL*, bool);

        virtual void BLOCK_OL(const MD_BLOCK_OL_DETAIL*, bool);

        virtual void BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool);

        virtual void BLOCK_HR(bool e);

        virtual void BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e, const markdown_config& cfg);

        virtual void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool);

        virtual void BLOCK_HTML(bool);

        virtual void BLOCK_P(bool);

        virtual void BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool);

        virtual void BLOCK_THEAD(bool, const markdown_config& cfg);

        virtual void BLOCK_TBODY(bool);

        virtual void BLOCK_TR(bool);

        virtual void BLOCK_TH(const MD_BLOCK_TD_DETAIL*, bool);

        virtual void BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool);

        virtual void SPAN_EM(bool e, const markdown_config& cfg);

        virtual void SPAN_STRONG(bool e, const markdown_config& cfg);

        virtual void SPAN_A(const MD_SPAN_A_DETAIL* d, bool e, const markdown_config& cfg);

        virtual void SPAN_IMG(const MD_SPAN_IMG_DETAIL*, bool);

        /**
         * @brief Called before and after inline code is rendered
         */
        virtual void render_inline_code(bool enter);

        virtual void SPAN_DEL(bool);

        virtual void SPAN_LATEXMATH(bool);

        virtual void SPAN_LATEXMATH_DISPLAY(bool);

        virtual void SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool);

        virtual void SPAN_U(bool);

        ////////////////////////////////////////////////////////////////////////////

        struct image_info {
            ImTextureID texture_id{};
            ImVec2 size;
            ImVec2 uv0;
            ImVec2 uv1;
            ImVec4 col_tint;
            ImVec4 col_border;
        };

        //use m_href to identify image
        virtual bool get_image(image_info& nfo) const;

        virtual bool make_font(ImFont** font, float& font_size, const markdown_config& config) const;

        [[nodiscard]] virtual ImVec4 get_color() const;

        //url == m_href
        virtual void open_url() const;

        //returns true if the term has been processed
        virtual bool render_entity(const char* str, const char* str_end);

        //returns true if the term has been processed
        virtual bool check_html(const char* str, const char* str_end);

        //called when '\n' in source text where it is not semantically meaningful
        virtual void soft_break();

        //e==true : enter
        //e==false : leave
        virtual void html_div(const std::string& dclass, bool e);

        ////////////////////////////////////////////////////////////////////////////

        //current state
        std::string m_href; //empty if no link/image

        bool m_is_underline = false;
        bool m_is_strikethrough = false;
        bool m_is_em = false;
        bool m_is_strong = false;
        bool m_is_table_header = false;
        bool m_is_image = false;
        bool m_is_code = false;
        bool m_is_inline_code = false;
        unsigned m_hlevel = 0; //0 - no heading

    private:
        int text(MD_TEXTTYPE type, const char* str, const char* str_end);

        int any_block(MD_BLOCKTYPE type, void* d, bool enter, const markdown_config& config);

        int span(MD_SPANTYPE type, void* d, bool e, const markdown_config& cfg);

        void render_text(const char* str, const char* str_end) const;

        bool render_alert(const char* str, const char* str_end);

        void set_font(bool e, const markdown_config& config);

        void set_color(bool e);

        void set_href(bool e, const MD_ATTRIBUTE& src);

        static void line(ImColor c, bool under);

        //table state
        bool m_table_context = false;
        bool m_table_active = false;
        unsigned m_table_index = 0;
        MD_ALIGN m_table_align = MD_ALIGN_DEFAULT;

        struct quote_info {
            ImVec2 start;
            bool alert_candidate = true;
        };

        std::vector<quote_info> m_quote_stack;

        //list state
        struct list_info {
            unsigned cur_ol;
            char delim;
            bool is_ol;
        };

        std::vector<list_info> m_list_stack;

        std::vector<std::string> m_div_stack;

        MD_PARSER m_md{};
    };
}