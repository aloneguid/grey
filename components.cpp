#include "components.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include <cstdarg>
#include <map>
#include <iostream>
#include <algorithm>
#include <fmt/core.h>
#include "../common/str.h"
#include "../common/stl.hpp"

using namespace std;

namespace grey
{
    void set_colours(emphasis em, ImColor& normal, ImColor& hovered, ImColor& active) {
        switch(em) {
            case emphasis::primary:
                normal = emphasis_primary_colour;
                hovered = emphasis_primary_colour_hovered;
                active = emphasis_primary_colour_hovered;
                break;
            case emphasis::error:
                normal = emphasis_error_colour;
                hovered = emphasis_error_colour_hovered;
                active = emphasis_error_colour_active;
                break;
            case emphasis::warning:
                normal = emphasis_warning_colour;
                hovered = emphasis_warning_colour_hovered;
                active = emphasis_warning_colour_active;
                break;
        }
    }

   static int incrementing_id;

   int generate_int_id() {
       return incrementing_id++;
   }

   static string generate_id(const string& prefix = "") {
       return prefix + std::to_string(incrementing_id++);
   }

   const void component::cursor_move(float x, float y) {
       ImVec2 mv = ImGui::GetCursorPos();
       mv.x += x;
       mv.y += y;
       ImGui::SetCursorPos(mv);
   }

   const void component::render() {

       if (on_frame) {
           on_frame(*this);
       }

       if(nullptr != is_visible) {
           if(*is_visible != is_visible_prev_frame) {
               is_visible_prev_frame = is_visible;

               if(on_visibility_changed) {
                   on_visibility_changed(*this);
               }
           }
           if(false == *is_visible) return;
       }


       bool change_width = !is_window && width != 0;
       if (change_width)
           ImGui::PushItemWidth(width);

       bool change_alpha = alpha != 1;
       if(change_alpha) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

       if(bg_draw) {
           start_cursor_pos = ImGui::GetCursorPos();
       }

       bool pad = padding_left != 0 || padding_top != 0;
       if(pad) {
           cursor_move(padding_left, padding_top);
       }

       render_visible();

       if(bg_draw) {
           ImGui::SetCursorPos(start_cursor_pos);
       }

       if(change_alpha) ImGui::PopStyleVar();

       if (change_width != 0)
           ImGui::PopItemWidth();

       if (on_hovered) {
           on_hovered(*this, ImGui::IsItemHovered());
       }

       if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
           //ImGui::BeginTooltip();
           //ImGui::Text(tooltip.c_str());
           //ImGui::EndTooltip();
           ImGui::SetTooltip("%s", tooltip.c_str());
       }

       state_click_pressed = (on_click && ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left));

       if (state_click_pressed) {
           if (!on_click_sent) {
               on_click_sent = true;
               on_click(*this);
           }
       } else {
           on_click_sent = false;
       }

   }

   const void common_component::render_visible() {
       switch(c) {
           case 1:
               ImGui::SameLine(arg1);
               break;
           case 2:
               ImGui::Spacing();
               break;
           case 3:
               ImGui::Separator();
               break;

       }
   }

   const void container::render() {
       // render non-owned children
       if(!managed_children.empty()) {
           for(const auto& mc : managed_children) {
               mc->render();
           }
       }

       component::render();
   }

   void component::set_emphasis(emphasis em)
   {
      this->em = em;
      set_colours(em, em_normal, em_hovered, em_active);
   }

   void container::clear() {
       owned_children_new.clear();
       is_dirty = true;
   }

   void container::assign_child(std::shared_ptr<component> child) {
       if(!is_dirty) {
           // make a copy
           owned_children_new = vector<shared_ptr<component>>{owned_children};
       }

       owned_children_new.push_back(child);
       child->parent = this;
       is_dirty = true;
   }

   void container::assign_managed_child(std::shared_ptr<component> child)
   {
      managed_children.push_back(child);
      child->parent = this;
   }

   std::shared_ptr<component> container::get_child(int index) {
       if (index >= owned_children.size()) return shared_ptr<component>();

       return owned_children[index];
   }

   bool container::move_child(shared_ptr<component> child, int pos, bool is_relative) {

       if(!is_dirty) {
           // make a copy
           owned_children_new = owned_children;
           is_dirty = true;
       }

       // we need child's index
       int idx = -1;
       for(int i = 0; i < owned_children_new.size(); i++) {
           if(owned_children_new[i]->id == child->id) {
               idx = i;
               break;
           }
       }
       if(idx == -1) return false;

       return stl::move(owned_children_new, idx, pos, is_relative);
   }

   std::shared_ptr<label> container::make_label(const std::string& text,
       const std::string& tooltip, bool is_enabled, bool same_line) {
       auto r = make_shared<label>(text, false);
       r->tooltip = tooltip;
       r->is_enabled = is_enabled;
       r->same_line = same_line;
       assign_child(r);
       return r;
   }


   std::shared_ptr<label> container::make_label(string* text, bool is_bullet) {
      auto r = make_shared<label>(text, is_bullet);
      assign_child(r);
      return r;
   }

   std::shared_ptr<menu_bar> container::make_menu_bar() {
       auto r = make_shared<menu_bar>();
       assign_child(r);
       return r;
   }

   std::shared_ptr<listbox> container::make_listbox(const string& label) {
       auto r = make_shared<listbox>(label);
       assign_child(r);
       return r;
   }

   std::shared_ptr<input> container::make_input(const string& label, string* value) {
       auto r = make_shared<input>(label, value);
       assign_child(r);
       return r;
   }

   std::shared_ptr<input_int> container::make_input_int(const string& label, int* value) {
       auto r = make_shared<input_int>(label, value);
       assign_child(r);
       return r;
   }

   std::shared_ptr<checkbox> container::make_checkbox(const string& label, bool* value) {
       auto r = make_shared<checkbox>(label, value);
       assign_child(r);
       return r;
   }

   std::shared_ptr<grey::tree> container::make_tree() {
       auto r = make_shared<tree>(tmgr);
       assign_child(r);
       return r;
   }

   std::shared_ptr<button> container::make_button(const string& label, bool is_small, emphasis e)
   {
      auto r = make_shared<button>(label, is_small, e);
      assign_child(r);
      return r;
   }

   std::shared_ptr<toggle> container::make_toggle(bool* value)
   {
      auto r = make_shared<toggle>(value);
      assign_child(r);
      return r;
   }

   std::shared_ptr<image> container::make_image_from_file(const string& path,
      size_t desired_width, size_t desired_height)
   {
      auto r = make_shared<image>(tmgr, path,
         nullptr, 0,
         desired_width, desired_height);
      assign_child(r);
      return r;
   }

   std::shared_ptr<image> container::make_image_from_memory(const string& tag,
      unsigned char* buffer, size_t buffer_length,
      size_t desired_width, size_t desired_height)
   {
      auto r = make_shared<image>(tmgr, tag, buffer, buffer_length, desired_width, desired_height);
      assign_child(r);
      return r;
   }

   std::shared_ptr<grey::big_table> container::make_big_table(std::vector<string> columns, size_t row_count)
   {
      auto r = make_shared<big_table>(columns, row_count);
      assign_child(r);
      return r;
   }

   std::shared_ptr<tabs> container::make_tabs(bool tab_list_popup) {
       auto r = make_shared<tabs>(tmgr, tab_list_popup);
       assign_child(r);
       return r;
   }

   std::shared_ptr<slider> container::make_slider(const string& label, float* value, float min, float max)
   {
      auto r = make_shared<slider>(label, value, min, max);
      assign_child(r);
      return r;
   }

   std::shared_ptr<progress_bar> container::make_progress_bar(float* value, const char* overlay_text, float height)
   {
      auto r = make_shared<progress_bar>(value, overlay_text, height);
      assign_child(r);
      return r;
   }

   std::shared_ptr<grey::modal_popup> container::make_modal_popup(const string& title)
   {
      auto r = make_shared<modal_popup>(tmgr, title);
      assign_child(r);
      return r;
   }

   std::shared_ptr<plot> container::make_plot(const string& title, size_t max_values) {
       auto r = make_shared<plot>(title, max_values);
       assign_child(r);
       return r;
   }

   std::shared_ptr<metrics_plot> container::make_metrics_plot(size_t max_points) {
       auto r = make_shared<metrics_plot>(max_points);
       assign_child(r);
       return r;
   }


   std::shared_ptr<status_bar> container::make_status_bar()
   {
      auto r = make_shared<status_bar>(tmgr);
      assign_child(r);
      //assign_managed_child(r);
      return r;
   }

   std::shared_ptr<accordion> container::make_accordion(const string& label) {
       auto r = make_shared<accordion>(tmgr, label, false);
       assign_child(r);
       return r;
   }

   std::shared_ptr<child> container::make_child_window(size_t width, size_t height, bool horizonal_scroll) {
       auto r = make_shared<child>(tmgr, width, height, horizonal_scroll);
       assign_child(r);
       return r;
   }

   std::shared_ptr<group> container::make_group() {
       auto r = make_shared<group>(tmgr);
       assign_child(r);
       return r;
   }

   std::shared_ptr<selectable> container::make_selectable(const std::string& value)
   {
      auto r = make_shared<selectable>(value);
      assign_child(r);
      return r;
   }

#if _DEBUG
   std::shared_ptr<demo> container::make_demo() {
       auto r = make_shared<demo>();
       assign_child(r);
       return r;
   }
#endif

   void container::set_pos(float x, float y, bool is_movement)
   {
      assign_child(make_shared<positioner>(x, y, is_movement));
   }

   void container::same_line(float offset_left)
   {
      assign_child(make_shared<common_component>(1, offset_left));
   }

   void container::spacer()
   {
      assign_child(make_shared<common_component>(2));
   }

   void container::separator()
   {
      assign_child(make_shared<common_component>(3));
   }

   std::shared_ptr<imgui_raw> container::make_raw_imgui() {
       auto r = make_shared<imgui_raw>();
       assign_child(r);
       return r;
   }

   const void container::render_children() {
       if(owned_children.empty()) return;
       for(auto child : owned_children) {
           child->render();
       }
   }

   void container::post_render() {

       if(is_dirty) {
           owned_children = owned_children_new;
           owned_children_new.clear();
           is_dirty = false;
       }

       for(auto child : owned_children) {
           child->post_render();
       }
   }

#ifdef _DEBUG
   const void demo::render_visible() {
       ImGui::ShowDemoWindow();
   }
#endif

   grey::child::child(grey_context& mgr, size_t width, size_t height, bool horizonal_scroll)
       : container{mgr}, size{static_cast<float>(width), static_cast<float>(height)} {
       if(horizonal_scroll)
           flags |= ImGuiWindowFlags_HorizontalScrollbar;
   }

   const void child::render_visible() {
       if(em != emphasis::none) {
           ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)em_normal);
       }

       ImVec2 tsz = size;

       if(padding_bottom != 0) {
           ImVec2 wsz = ImGui::GetWindowSize();
           tsz = ImVec2(tsz.x, wsz.y - padding_bottom);
       }

       if(ImGui::BeginChild(this->id.c_str(), tsz, has_border, flags)) {
           render_children();
       }
       ImGui::EndChild();   // must be called regardless

       if(em != emphasis::none) {
           ImGui::PopStyleColor();
       }
   }

   group::group(grey_context& mgr) : container{mgr} {

   }

   const void group::render_visible() {

       // group does not follow begin/end common rules. Always call begin. Always call end.
       ImGui::BeginGroup();

       render_children();

       // dirty hack, but works!
       if(spread_horizontally) {
           auto sz = ImGui::GetWindowSize();
           ImGui::SameLine(sz.x - 10 * scale); ImGui::Text("");
           ImGui::SameLine();
       }

       ImGui::EndGroup();

       auto& style = ImGui::GetStyle();

       if(border_colour) {
           auto min = ImGui::GetItemRectMin();
           auto max = ImGui::GetItemRectMax();
           ImDrawList* draw_list = ImGui::GetWindowDrawList();

           //draw_list->AddLine(min, ImVec2(min.x, max.y), (ImU32)emphasis_primary_colour);
           draw_list->AddRect(min, max, border_colour, style.FrameRounding);
       }

       if(ImGui::IsItemHovered() && hover_border_colour.o > 0) {
           auto min = ImGui::GetItemRectMin();
           auto max = ImGui::GetItemRectMax();
           ImDrawList* draw_list = ImGui::GetWindowDrawList();

           //draw_list->AddLine(min, ImVec2(min.x, max.y), (ImU32)emphasis_primary_colour);
           draw_list->AddRect(min, max, hover_border_colour, style.FrameRounding);
       }

       if(ImGui::IsItemHovered() && hover_bg_colour.o > 0) {
           auto min = ImGui::GetItemRectMin();
           auto max = ImGui::GetItemRectMax();
           ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

           //draw_list->AddLine(min, ImVec2(min.x, max.y), (ImU32)emphasis_primary_colour);
           //draw_list->AddRect(min, max, hover_border_colour);
           draw_list->AddRectFilled(min, max, hover_bg_colour, style.FrameRounding);
       }

       //if(on_click && ImGui::IsItemClicked()) {
       //    on_click(*this);
       //}

       if(on_hovered) {
           on_hovered(*this, ImGui::IsItemHovered());
       }
   }

   const void grey::status_bar::render_visible() {
       // source: https://github.com/ocornut/imgui/issues/3518#issuecomment-807398290
       ImGuiViewport* win_vp = ImGui::GetWindowViewport();
       //ImGuiViewport* main_vp = ImGui::GetMainViewport();
       float height = ImGui::GetFrameHeight();
       if(ImGui::BeginViewportSideBar("##MainStatusBar", win_vp, ImGuiDir_Down, height, flags)) {
           if(ImGui::BeginMenuBar()) {
               render_children();
               ImGui::EndMenuBar();
           }

       }
       ImGui::End(); // status bar, should be outside of BeginViewportSideBar just like for a normal window
   }

   accordion::accordion(grey_context& mgr, const string& label, bool is_closeable) : container{mgr}, is_closeable{is_closeable} {
       this->label = sys_label(label);
   }

   const void grey::accordion::render_visible() {
       if(is_closeable ? ImGui::CollapsingHeader(label.c_str(), &is_open) : ImGui::CollapsingHeader(label.c_str())) {
           render_children();
       }
   }

   void window::close() {
       is_open = false;
   }

   grey::window::window(grey_context& ctx, string title, float width, float height)
       : container{ctx}, title{title}, id_title{ sys_label(title) },
       ctx{ctx} {

       is_window = true;

       float scale = get_system_scale();
       component::width = width * scale;
       component::height = height * scale;

       //flags = ImGuiWindowFlags_NoCollapse;
       flags = ImGuiWindowFlags_NoBringToFrontOnFocus;

       /*if(is_maximized) {
           flags = (
              ImGuiWindowFlags_NoBringToFrontOnFocus |
              //ImGuiWindowFlags_NoCollapse |
              ImGuiWindowFlags_NoSavedSettings
              //ImGuiWindowFlags_NoTitleBar |
              //ImGuiWindowFlags_NoResize
              );

           // need to call PopStyleVar elsewhere
           //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
       } else {
           flags = 0;
           //flags |= ImGuiWindowFlags_NoDocking;
       }*/

       //if(!show_title) flags |= ImGuiWindowFlags_NoTitleBar;

       if(has_menu_space) flags |= ImGuiWindowFlags_MenuBar;
   }

   window::~window() {
#if _DEBUG
       cout << "window destroyed" << endl;
#endif
   }

   const void window::render_visible() {

       ImGuiWindowFlags rflags = flags;
       if(has_menu_space) rflags |= ImGuiWindowFlags_MenuBar;
       if(!can_resize) rflags |= ImGuiWindowFlags_NoResize;

       if(!initialised) {
           if(width != 0 && height != 0) {
               ImGui::SetNextWindowSize(ImVec2(width, height));
           }

           initialised = true;
       }

       // the rest

       if(change_pos) {
           ImGui::SetNextWindowPos(change_pos_point);
           change_pos = false;
       }


       bool began = ImGui::Begin(id_title.c_str(), &is_open, rflags);

       /*if(is_dockspace) {
           // useful docking links:
           // - https://github.com/ocornut/imgui/issues/4430
           // - https://gist.github.com/moebiussurfing/d7e6ec46a44985dd557d7678ddfeda99
           if (first_time) {
               first_time = false;

               dockspace_id = ImGui::GetID("MainDockspace");
           }

           ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
       }*/

       if (began) {
           ImVec2 pos = ImGui::GetWindowPos();
           ImVec2 size = ImGui::GetWindowSize();
           width = size.x;
           height = size.y;

           if(do_top) {
               auto vp = ImGui::GetWindowViewport();
               if(vp->PlatformWindowCreated) {
                   ctx.bring_native_window_to_top(vp->PlatformHandleRaw);
                   do_top = false;
               }
           }

           if(do_center) {
               ImVec2 ss = ImGui::GetPlatformIO().Monitors[0].WorkSize;
               ImVec2 ws = ImGui::GetWindowSize();
               change_pos = true;
               change_pos_point = ImVec2(ss.x / 2 - ws.x / 2, ss.y / 2 - ws.y / 2);
               do_center = false;
           }

           render_children();
       }

       if(is_open != was_open) {
           was_open = is_open;

           if(on_open_changed) {
               on_open_changed(is_open);
           }

           if(!is_open && detach_on_close) {
               ctx.detach(this->id);
           }
       }

       // for the window, End must be called regardless
       ImGui::End();

       /*if(is_maximized) {
           ImGui::PopStyleVar();
       }*/
   }

   void window::center() {
       do_center = true;
   }

   void window::bring_to_top() {
       do_top = true;
   }

   button::button(const string& label, bool is_small, emphasis e) : is_small{is_small} {
       set_label(label);
       set_emphasis(e);
   }

   void button::set_label(const string& label)
   {
      this->label = sys_label(label);
   }

   const void button::render_visible() {
       bool disabled = !is_enabled;  // cache value as it might change after callback

       if(disabled) {
           ImGui::BeginDisabled(true);
       }

       if(em != emphasis::none) {
           ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)em_normal);
           ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)em_hovered);
           ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)em_active);
       }

       if(is_small) {
           if(ImGui::SmallButton(label.c_str()) && on_pressed) {
               on_pressed(*this);
           }
       } else {
           if(ImGui::Button(label.c_str()) && on_pressed) {
               on_pressed(*this);
           }
       }

       if(disabled) ImGui::EndDisabled();
       if(em != emphasis::none) ImGui::PopStyleColor(3);
   }

   const void grey::toggle::render_visible() {
       if(*value) {
           if(!label_off.empty()) {
               ImGui::Text(label_off.c_str());
               ImGui::SameLine();
           }
       } else {
           if(!label_on.empty()) {
               ImGui::Text(label_on.c_str());
               ImGui::SameLine();
           }
       }

       ImVec4* colors = ImGui::GetStyle().Colors;
       ImVec2 p = ImGui::GetCursorScreenPos();
       ImDrawList* draw_list = ImGui::GetWindowDrawList();

       float height = ImGui::GetFrameHeight();
       float width = height * 1.55f;
       float radius = height * 0.50f;

       ImGui::InvisibleButton(id.c_str(), ImVec2(width, height));
       if(ImGui::IsItemClicked()) {
           *value = !*value;
           if(on_toggled) on_toggled(*value);
       }

       ImGuiContext& gg = *GImGui;
       float ANIM_SPEED = 0.085f;
       if(gg.LastActiveId == gg.CurrentWindow->GetID(id.c_str()))// && g.LastActiveIdTimer < ANIM_SPEED)
           float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
       if(ImGui::IsItemHovered())
           draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*value ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
       else
           draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*value ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
       draw_list->AddCircleFilled(ImVec2(p.x + radius + (*value ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
   }

   string label::get_value() {
       if(value_ptr)
           return *value_ptr;

       return value;
   }

   void label::set_value(const string& value) {
       // only allow changing it if we own it!
       if(!value_ptr)
           this->value = value;
   }

   const void label::render_visible() {

       if(same_line) {
           ImGui::SameLine();
       }

       if(text_wrap_pos > 0)
           ImGui::PushTextWrapPos(text_wrap_pos);

       const char* buf = value_ptr == nullptr ? value.c_str() : (*value_ptr).c_str();

       if(is_bullet) {
           ImGui::Bullet();
       }

       if(is_enabled) {
           if(em != emphasis::none) {
               ImGui::TextColored(em_normal, buf);
           } else {
               ImGui::Text(buf);
           }
       } else {
           ImGui::TextDisabled("%s", buf);
       }

       if(text_wrap_pos > 0)
           ImGui::PopTextWrapPos();
   }

   selectable::selectable(const std::string& value)
       : value{sys_label(value)} {

   }

   const void selectable::render_visible() {
       // todo: flag can be set to appear as disabled
       //ImGuiSelectableFlags rflags = flags;

       bool align = alignment.x > 0 || alignment.y > 0;

       if(align)
           ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);

       if(ImGui::Selectable(value.c_str(), &is_selected, flags, size)) {
           if(on_selected) {
               on_selected(*this);
           }
       }

       if(align)
           ImGui::PopStyleVar();
   }

   shared_ptr<menu_item> menu_item::add(const std::string& id, const std::string& label, const std::string& icon) {
       auto mi = make_shared<menu_item>(id, label, icon);
       children.push_back(mi);
       return mi;
   }

   const void menu_bar::render_visible() {
       bool rendered = is_main_menu ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();

       if(rendered) {
           for(auto mi : root->children) {
               render(mi, false);
           }

           if(is_main_menu)
               ImGui::EndMainMenuBar();
           else
               ImGui::EndMenuBar();
       }
   }

   void menu_bar::render(shared_ptr<menu_item> mi, bool icon_pad) {

       if(mi->label == "-") {
           ImGui::Separator();
           return;
       }

       string label = icon_pad
           ? string{"       "} + mi->label
           : mi->label;

       ImVec2 cp = ImGui::GetCursorPos();

       if(mi->children.empty()) {
           if(ImGui::MenuItem(label.c_str(),
               mi->shortcut_text.empty() ? nullptr : mi->shortcut_text.c_str(),
               mi->is_selected, mi->is_enabled) && clicked && !mi->id.empty()) {
               clicked(*mi);
           }
       } else {
           if(ImGui::BeginMenu(label.c_str())) {
               bool has_icons{false};
               for(auto imi : mi->children) {
                   if(!imi->icon.empty()) {
                       has_icons = true;
                       break;
                   }
               }

               for(auto imi : mi->children) {
                   render(imi, has_icons);
               }

               ImGui::EndMenu();
           }
       }

       if(!mi->icon.empty()) {
           ImGui::SetCursorPos(cp);
           ImGui::Text(mi->icon.c_str());
       }
   }

   component::component(const string& id) {
       this->id = id.empty() ? generate_id() : id;
   }

   listbox::listbox(const string& title) {
       this->title = sys_label(title);
   }

   const void listbox::render_visible() {
       ImVec2 size = is_full_width
           ? ImVec2(-FLT_MIN, items_tall * ImGui::GetTextLineHeightWithSpacing())
           : ImVec2(0, items_tall * ImGui::GetTextLineHeightWithSpacing());

       // begin rendering
       bool ok;
       switch(mode) {
           case grey::listbox_mode::list:
           {
               ok = ImGui::BeginListBox(title.c_str(), size);
           }
           break;
           case grey::listbox_mode::combo:
           {
               string preview = (selected_index == -1) ? "" : items[selected_index].text;
               ok = ImGui::BeginCombo(title.c_str(), preview.c_str());
           }
           break;
           case grey::listbox_mode::icons:
           {
               ok = true;
           }
           break;
           default:
               ok = false;
               break;
       }

       // render items
       if(ok) {
           size_t si = 0;

           for(list_item& item : items) {
               bool is_selected = selected_index == si;

               switch(mode) {
                   case grey::listbox_mode::list:
                   case grey::listbox_mode::combo:
                   {
                       if(ImGui::Selectable(item.text.c_str(), is_selected)) {
                           selected_index = si;
                           if(on_selected) {
                               on_selected(si, item);
                           }
                       }

                       if(is_selected)
                           ImGui::SetItemDefaultFocus();
                   }
                   break;
                   case grey::listbox_mode::icons:
                   {
                       if(si > 0) ImGui::SameLine();

                       if(is_selected) {
                           ImGui::Text(item.text.c_str());
                       } else {
                           ImGui::TextDisabled(item.text.c_str());

                           // show "hand" cursor for disabled (selectable) options
                           if(ImGui::IsItemHovered()) {
                               ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                               // check if mouse is clicked on this item
                               if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                                   selected_index = si;
                                   if(on_selected) {
                                       on_selected(si, item);
                                   }
                               }
                           }

                       }
                   }
                   break;
               }

               if(!item.tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                   ImGui::SetTooltip(item.tooltip.c_str());
               }

               si += 1;
           }

           // end rendering
           switch(mode) {
               case grey::listbox_mode::list:
                   ImGui::EndListBox();
                   break;
               case grey::listbox_mode::combo:
                   ImGui::EndCombo();
                   break;
               default:
                   break;
           }

       }
   }

   // custom tree node
   // https://github.com/ocornut/imgui/issues/282#issuecomment-135988273
   bool tree_node::x_tree_node() {
       const ImGuiStyle& style = ImGui::GetStyle();
       ImGuiStorage* storage = ImGui::GetStateStorage();

       bool x_opened = false;

       float x = ImGui::GetCursorPosX();
       ImGui::BeginGroup();
       /*
       if (ImGui::InvisibleButton(label.c_str(), ImVec2(-1, ImGui::GetFontSize() + style.FramePadding.y * 2)))
       {
          int* p_opened = storage->GetIntRef(id, 0);
          opened = *p_opened = !*p_opened;
       }*/

       ImGui::InvisibleButton(id.c_str(), ImVec2(-1, ImGui::GetFontSize() + style.FramePadding.y * 2));

       bool hovered = ImGui::IsItemHovered();
       bool active = ImGui::IsItemActive();
       if(hovered || active)
           ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
              ImColor(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

       // Icon, text
       ImGui::SameLine(x);
       //ImGui::ColorButton(x_opened ? ImColor(255, 0, 0) : ImColor(0, 255, 0));
       //ImGui::SameLine();
       ImGui::Text(label.c_str());
       ImGui::EndGroup();
       if(x_opened)
           ImGui::TreePush(label.c_str());
       return x_opened;
   }

   const void tree_node::render_visible() {
       ImGuiTreeNodeFlags flags =
           ImGuiTreeNodeFlags_SpanAvailWidth;

       if(is_bold) flags |= ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
       if(is_expanded) flags |= ImGuiTreeNodeFlags_DefaultOpen;
       if(is_leaf) flags |= ImGuiTreeNodeFlags_Leaf;


       if(em != emphasis::none) {
           ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)em_normal);
           ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)em_hovered);
           ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)em_active);
       }

       //ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 0, 0, 1));
       // use overload that allows passing node ID instead of just label, so node persists on rename
       if(ImGui::TreeNodeEx(id.c_str(), flags, label.c_str()))
           //if (x_tree_node())
       {
           render_children();

           for(auto child : nodes) {
               child->render();
           }

           ImGui::TreePop();
       }

       if(em != emphasis::none) ImGui::PopStyleColor(3);
   }

   std::shared_ptr<tree_node> tree_node::add_node(const string& label, bool is_expanded, bool is_leaf) {
       auto node = make_shared<tree_node>(tmgr, label, is_expanded, is_leaf);
       nodes.push_back(node);
       return node;
   }

   std::shared_ptr<tree_node> tree::add_node(const string& label, bool is_expanded, bool is_leaf) {
       auto node = make_shared<tree_node>(tmgr, label, is_expanded, is_leaf);
       nodes.push_back(node);
       return node;
   }

   void tree::remove_node(size_t index) {
       nodes.erase(nodes.begin() + index);
   }

   const void tree::render_visible() {
       for(auto node : nodes) {
            node->render();
       }
   }

   void tree::post_render() {
       for(auto node : nodes) {
           node->post_render();
       }
   }

   input::input(const string& label, string* value)
       : value{value} {
       //buffer.resize(buf_size);
       owns_mem = value == nullptr;
       if(owns_mem) {
           this->value = new string();
       }

       this->label = sys_label(label);
   }

   input::~input() {
       if(owns_mem) {
           delete value;
       }
   }

   const void input::render_visible() {
       bool is_error = is_in_error_state;
       if(is_error)
           ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.00f));

       if(!is_enabled) ImGui::BeginDisabled();

       if(lines <= 1) {
           if(ImGui::InputText(label.c_str(), value, flags)) {
               fire_changed();
           }
       } else {
           ImVec2 size;

           if(fill && parent) {
               size = ImGui::GetWindowSize();
               size.x = -FLT_MIN;
               size.y -= ImGui::GetTextLineHeight() * (3 + lines_bottom_padding);
           } else {
               size = ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * lines);
           }

           if(ImGui::InputTextMultiline(label.c_str(), value,
               size,
               flags)) {
               fire_changed();
           }
       }
       if(!is_enabled) ImGui::EndDisabled();
       //ImGui::InputText(label.c_str(), &buffer[0], buf_size);

       if(is_error)
           ImGui::PopStyleColor();

       if(ImGui::IsItemFocused()) {
           bool key_arrow_up_pressed_now = ImGui::IsKeyDown(ImGuiKey_UpArrow);
           if(!key_arrow_up_pressed_now && key_arrow_up_pressed && on_arrow_up) {
               on_arrow_up();
           }
           key_arrow_up_pressed = key_arrow_up_pressed_now;

           bool key_arrow_down_pressed_now = ImGui::IsKeyDown(ImGuiKey_DownArrow);
           if(!key_arrow_down_pressed_now && key_arrow_down_pressed && on_arrow_down) {
               on_arrow_down();
           }
           key_arrow_down_pressed = key_arrow_down_pressed_now;
       }
   }

   void input::fire_changed() {
       if(on_value_changed) {
           on_value_changed(*value);
       }
   }

   input_int::input_int(const string& label, int* value) : value{value} {
       owns_mem = value == nullptr;
       if(owns_mem) {
           this->value = new int();
       }

       this->label = sys_label(label);
   }

   input_int::~input_int() {
       if(owns_mem) {
           delete value;
       }
   }

   const void input_int::render_visible() {

       if(ImGui::InputInt(label.c_str(), value, step)) {
           if(on_value_changed) {
               on_value_changed(*value);
           }
       }
   }

   /*   void plot::add_point(float y)
   {
      x += ImGui::GetIO().DeltaTime;
      buffer.AddPoint(x, y);
   }
   */

   /*node_editor::node_editor()
   {
      ImNodes::CreateContext();
   }

   node_editor::~node_editor()
   {
      ImNodes::DestroyContext();
   }

   const void node_editor::render_visible()
   {
      ImNodes::BeginNodeEditor();

      vector<int> attrs;


      for (int i = 1; i < 10; i++)
      {
         int node_id = generate_int_id();
         ImNodes::BeginNode(i);

         int id_out = generate_int_id();
         ImNodes::BeginOutputAttribute(id_out, ImNodesPinShape_Triangle);
         ImNodes::EndOutputAttribute();

         ImNodes::BeginNodeTitleBar();
         ImGui::TextUnformatted("title");
         ImNodes::EndNodeTitleBar();

         ImGui::Dummy(ImVec2(80.0f, 45.0f));

         ImNodes::EndNode();

         if (i > 1)
         {
            attrs.push_back(id_out);
         }
      }

      for (int i = 1; i < attrs.size(); i++)
      {
         ImNodes::Link(generate_int_id(), attrs[i], attrs[i - 1]);
      }

      if (minimap)
      {
         ImNodes::MiniMap();
      }

      ImNodes::EndNodeEditor();
   }*/

   void image::ensure_texture()
   {
      // textures must be cached in the backend

      //auto tag_e = texture_cache.find(file_path_or_tag);
      //if (tag_e == texture_cache.end())
      //{
      /*
         int width, height;
         void* texture = buffer == nullptr
            ? mgr.load_texture_from_file(file_path_or_tag, width, height)
            : mgr.load_texture_from_memory(buffer, len, width, height);
         texture_cache[file_path_or_tag] = texture_tag{
            texture,
            static_cast<size_t>(width),
            static_cast<size_t>(height) };*/
      //}
   }

   image::image(const grey_context& mgr,
      const string& file_path,
      unsigned char* buffer, unsigned int len,
      size_t desired_width, size_t desired_height) :
       file_path_or_tag{file_path},
       buffer{buffer}, len{len},
       mgr{mgr},
       desired_width{desired_width}, desired_height{desired_height} {
       int width, height;
       grey_context& ncmgr = const_cast<grey_context&>(mgr);
       texture = (buffer == nullptr)
           ? ncmgr.load_texture_from_file(file_path, width, height)
           : ncmgr.load_texture_from_memory(file_path, buffer, len, width, height);

       actual_width = width;
       actual_height = height;
       this->desired_width = desired_width == string::npos ? actual_width : desired_width;
       this->desired_height = desired_height == string::npos ? actual_height : desired_height;
   }

   const void image::render_visible() {
       if(texture) {
           if(rounding == 0) {
               ImGui::Image(texture, ImVec2(desired_width, desired_height));
           } else {

               ImDrawList* dl = ImGui::GetForegroundDrawList();
               ImVec2 p_min = ImGui::GetCursorScreenPos();
               ImVec2 p_max = ImVec2(p_min.x + desired_width, p_min.y + desired_height);
               dl->AddImageRounded(texture, p_min, p_max,
                   ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                   rounding);
           }
       }
   }

   slider::slider(const string& label, float* value, float min, float max) :
      label{ label },
      value{ value },
      min{ min }, max{ max }
   {
   }

   slider::~slider()
   {
   }

   void slider::set_limits(float min, float max)
   {
      this->min = min;
      this->max = max;
   }

   const void slider::render_visible() {
       if(ImGui::SliderFloat(label.c_str(), value, min, max, format, flags)) {
           value_changed = true;
       }

       if(value_changed && !ImGui::IsAnyMouseDown()) {
           if(on_value_changed) {
               on_value_changed(*this);
           }

           value_changed = false;
       }

   }

   progress_bar::progress_bar(float* value, const char* overlay_text, float height)
      : value{ value }, overlay_text{ overlay_text }, size{0, height}
   {

   }

   const void progress_bar::render_visible()
   {
      ImGui::ProgressBar(*value, size, overlay_text);
   }

   modal_popup::modal_popup(grey_context& mgr, const string& title) : container{mgr}, title{title} {
       // pop-ups should be invisible initially
       if(is_visible) {
           *is_visible = false;
       }
   }

   const void modal_popup::render() {
       // to open, call ImGui::OpenPopup(id.c_str()); _once_

       // to close, call CloseCurrentPopup _once_

       // do not fuck up this flag flipping logic, it's very fragile!!!!

       if(!sys_open) {  // check sys flip state - set to false by ImGui when popup closure is handled by imgui itself
           *is_visible = false;
           was_visible = false;
           sys_open = true;
       }

       if(!was_visible && *is_visible) {
           // quickly show and flip the flag

           sys_open = true;

           ImGui::OpenPopup(title.c_str());

           was_visible = true;
       }

       container::render();

       //render_visible();
   }

   const void modal_popup::render_visible() {
       if(ImGui::BeginPopupModal(title.c_str(), &sys_open)) {
           render_children();

           if(was_visible && !is_visible) {
               // "close" needs to be called from inside the pop-up cycle, otherwise it won't work
               ImGui::CloseCurrentPopup();

               was_visible = false;
           }

           ImGui::EndPopup();
       }
   }

   checkbox::checkbox(const string& text, bool* value) : text{ text }, value{ value }
   {
      owns_mem = value == nullptr;
      if (value == nullptr)
         this->value = new bool();
   }

   checkbox::~checkbox() {
      if (owns_mem) {
         delete value;
      }
   }

   const void checkbox::render_visible() {
       if(is_highlighted) {
           ImGui::PushStyleColor(
              ImGuiCol_CheckMark,
              (ImVec4)ImColor::HSV(5.0f, 255.0f, 255.0f));
       }

       if(em != emphasis::none) {
           ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)em_normal);
       }

       if(render_as_icon) {
           if(*value) {
               ImGui::Text(text.c_str());
           } else {
               ImGui::TextDisabled(text.c_str());
           }

           if(ImGui::IsItemHovered()) {
               ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

               if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                   if(!mouse_was_down) {
                       *value = !*value;
                       if(on_value_changed) {
                           on_value_changed(*value);
                       }
                   }
                   mouse_was_down = true;
               } else {
                   mouse_was_down = false;
               }
           }

       } else {
           if(ImGui::Checkbox(sys_label(text).c_str(), value)) {
               if(on_value_changed)
                   on_value_changed(*value);
           }
       }

       if(is_highlighted) {
           ImGui::PopStyleColor();
       }

       if(em != emphasis::none) {
           ImGui::PopStyleColor();
       }
   }

   const void table::render_visible()
   {
      if (data.empty()) return;

      size_t v_size = std::min(data[0].size(), MAX_TABLE_COLUMNS);
      if (v_size == 0) return;

      // render table
      if (ImGui::BeginTable(id.c_str(), v_size,
         ImGuiTableFlags_RowBg |
         ImGuiTableFlags_BordersV |
         ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
         ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
         ))
      {
         // render header

         if (has_header)
         {
            ImGui::TableSetupScrollFreeze(0, 1);   // freeze top row

            for(int i = 0; i < std::min(data[0].size(), v_size); i++)
            {
               ImGui::TableSetupColumn(data[0][i].c_str());
            }
            ImGui::TableHeadersRow();
         }

         // render body

         for (int i = has_header ? 1 : 0; i < data.size(); i++)
         {
            ImGui::TableNextRow();
            auto& row = data[i];

            for (int ic = 0; ic < std::min(row.size(), v_size); ic++)
            {
               ImGui::TableSetColumnIndex(ic);

               ImGui::TextUnformatted(row[ic].c_str());
            }
         }

         ImGui::EndTable();
      }
   }

   const void grey::table_cell::render_visible() {
       render_children();
   }

   void table::set_data(std::vector<std::vector<string>> data, bool has_header)
   {
      this->data = data;
      this->has_header = has_header;
   }

   const void big_table::render_visible()
   {
      // tables: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

      const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
      ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
      if (ImGui::BeginTable(id.c_str(), columns.size(),
         //ImGuiTableFlags_RowBg |
         ImGuiTableFlags_BordersV |
         //ImGuiTableFlags_SizingFixedFit |
         //ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter |
         //ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
         //ImGuiTableFlags_ScrollX |
         ImGuiTableFlags_ScrollY))
      {
         ImGui::TableSetupScrollFreeze(0, 1);   // freeze top row

         for (const auto& c : columns)
         {
            ImGui::TableSetupColumn(c.c_str(), ImGuiTableColumnFlags_WidthStretch);
         }
         ImGui::TableHeadersRow();

         ImGuiListClipper clipper;
         clipper.Begin(row_count);
         while (clipper.Step())
         {
            vector<string> row;
            for (int i_row = clipper.DisplayStart; i_row < clipper.DisplayEnd; i_row++)
            {
               ImGui::TableNextRow();
               if (get_row_data)
               {
                  get_row_data(*this, i_row, row);

                  for (int col = 0; col < row.size(); col++)
                  {
                     if (ImGui::TableSetColumnIndex(col))
                     {
                        ImGui::Text(row[col].c_str());
                        //ImGui::Selectable(row[col].c_str(), &selected);
                     }
                  }
               }
            }
         }

         ImGui::EndTable();
      }
   }

   tabs::tabs(grey_context& mgr, bool tab_list_popup) : mgr{mgr} {
       flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_AutoSelectNewTabs;
       if(tab_list_popup)
           flags |= ImGuiTabBarFlags_TabListPopupButton;
   }

   const void tabs::render_visible() {
       if(tab_headers.empty()) return;

       if(ImGui::BeginTabBar(id.c_str(), flags)) {

           for(size_t i = 0; i < tab_headers.size(); i++) {

               // handle cases when there can be more than one tab header with the same title
               string tl = fmt::format("{}##{}", tab_headers[i], i);

               if(ImGui::BeginTabItem(tl.c_str())) {
                   // it's selected

                   tab_containers[i]->render_visible();

                   if(on_tab_changed && i != rendered_selected_idx) {
                       on_tab_changed(i);
                   }

                   rendered_selected_idx = i;

                   ImGui::EndTabItem();
               }

           }
           ImGui::EndTabBar();
       }
   }

   void tabs::post_render() {

       component::post_render();

       if(tabs_dirty) {
           tab_headers = tab_headers_new;
           tab_headers_new.clear();
           tab_containers = tab_containers_new;
           tab_containers_new.clear();
           tabs_dirty = false;
       }

       for(auto tc : tab_containers) {
           tc->post_render();
       }
   }

   void tabs::clear() {
       tab_headers_new.clear();
       tab_containers_new.clear();
       tabs_dirty = true;
       rendered_selected_idx = -1;
   }

   std::shared_ptr<tab> tabs::make(const std::string& title) {

       if(!tabs_dirty) {
           tab_headers_new = tab_headers;
           tab_containers_new = tab_containers;
       }

       auto c = make_shared<tab>(mgr);
       tab_headers_new.push_back(title);
       tab_containers_new.push_back(c);
       tabs_dirty = true;
       return c;
   }

   const void hex_editor::render_visible() {
       me.DrawContents((void*)(&data[0]), data.size());
   }

   const void plot::render_visible() {
       if(values.empty()) return;

       ImVec2 ws = ImGui::GetWindowSize();
       ImGuiStyle& style = ImGui::GetStyle();

       //if(stick_to_bottom) {
       //    ImGui::SetCursorPosY(ws.y - height - style.ItemSpacing.y * 2);
       //}

       ImVec2 size = ImVec2(width == -1
          ? (ws.x - style.ItemSpacing.x * 2)
          : width,
          height);

       ImGui::PlotHistogram(
           label.c_str(),
           &values[0], values.size(), 0,
           plot_overlay_text.c_str(), // overlay text
           min, max,
           size);
   }

   void plot::add(float value) {
       if(values.size() == max_values) {
           values.erase(values.begin(), values.begin() + 1);
       }
       values.push_back(value);
       min = sticky_min == -1 ? *std::min_element(values.begin(), values.end()) : sticky_min;
       max = *std::max_element(values.begin(), values.end());
       if(min == max) min = 0;
   }

   void plot::set_label(const string& label) {
       this->label = label;
   }

   int MetricFormatter(double value, char* buff, int size, void* data) {
       const char* unit = (const char*)data;
       static double v[] = {1000000000,1000000,1000,1,0.001,0.000001,0.000000001};
       static const char* p[] = {"G","M","k","","m","u","n"};
       if(value == 0) {
           return snprintf(buff, size, "0 %s", unit);
       }
       for(int i = 0; i < 7; ++i) {
           if(fabs(value) >= v[i]) {
               return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
           }
       }
       return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
   }

   int NFormatter(double value, char* buff, int size, void* data) {
       string sv = fmt::format("# {}", (int)value);
       return strcpy_s(buff, size, sv.c_str());
   }


   int MemoryFormatter(double value, char* buff, int size, void* data) {
       string sv = str::to_human_readable_size(value);
       return strcpy_s(buff, size, sv.c_str());
   }

   int PercentageFormatter(double value, char* buff, int size, void* data) {
       string sv = fmt::format("{:.2f} %", value);
       return strcpy_s(buff, size, sv.c_str());
   }


   const void metrics_plot::render_visible() {

       ImPlot::SetNextAxesToFit();

       if(ImPlot::BeginPlot(id.c_str(), ImVec2(-1, height), ImPlotFlags_NoTitle)) {

           //ImPlot::SetupAxes("time", "# proc");
           //ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels);
           ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_AutoFit);
           ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);

           // setup stage (ImPlot doesn't allow to setup another axis after drawing)

           // Y1 - number of processes.
           if(!number_plots.empty()) {

               // get max across all plots
               double y_max{0};
               for(auto& def : number_plots) {
                   if(def.data.y_max_ever > y_max)
                       y_max = def.data.y_max_ever;
               }
               if(y_max > 0) y_max += y_max / 5;

               ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
               ImPlot::SetupAxisFormat(ImAxis_Y1, NFormatter);
               //ImPlot::SetupAxisLimits(ImAxis_Y1, 0, y_max);
           }

           if(!memory_plots.empty()) {
               ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_LockMin);
               //ImPlot::SetupAxisLimits(ImAxis_Y2, 0, y_max);
               ImPlot::SetupAxisFormat(ImAxis_Y2, MemoryFormatter);
           }

           if(!perc_plots.empty()) {
               ImPlot::SetupAxis(ImAxis_Y3, nullptr, ImPlotAxisFlags_LockMin);
               //ImPlot::SetupAxisLimits(ImAxis_Y2, 0, y_max);
               ImPlot::SetupAxisFormat(ImAxis_Y3, PercentageFormatter);
           }


           // draw stage

           //ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 5);

           if(!number_plots.empty()) {
               ImPlot::SetAxis(ImAxis_Y1);

               for(auto& def : number_plots) {
                   if(def.data.size() == 0) continue;

                   ImPlot::PlotLine(def.label.c_str(), def.data.x_begin(), def.data.y_begin(), def.data.size());
               }
           }

           if(!memory_plots.empty()) {
               ImPlot::SetAxis(ImAxis_Y2);

               for(auto& def : memory_plots) {
                   if(def.data.size() == 0) continue;

                   ImPlot::PlotLine(def.label.c_str(), def.data.x_begin(), def.data.y_begin(), def.data.size());
               }
           }

           if(!perc_plots.empty()) {
               ImPlot::SetAxis(ImAxis_Y3);

               for(auto& def : perc_plots) {
                   if(def.data.size() == 0) continue;

                   ImPlot::PlotLine(def.label.c_str(), def.data.x_begin(), def.data.y_begin(), def.data.size());
               }
           }

           //ImPlot::PopStyleVar();

           //ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AutoFit);
           //ImPlot::SetupAxisFormat

           /*for(auto& def : plots) {
               if(def.data.size() == 0) continue;

               //set axis index here
               //ImPlot::SetAxis(ImAxis)

               switch(def.axis_idx) {
                   case 0:
                       ImPlot::SetAxis(ImAxis_Y1);
                       break;
                   case 1:
                       ImPlot::SetAxis(ImAxis_Y2);
                       break;
               }

               switch(def.type) {
                   case implot_type::bar:
                       ImPlot::PlotBars(def.label.c_str(),
                           def.data.x_begin(), def.data.y_begin(), def.data.size(), 
                           0.7, 1);
                       break;
                   case implot_type::line:
                       ImPlot::PlotLine(def.label.c_str(),
                           def.data.x_begin(), def.data.y_begin(), def.data.size());
                       break;
               }
           }
           */
           //ImPlot::PlotLine("#proc1", buf.begin(), buf.size());

           ImPlot::EndPlot();
       }
   }

   positioner::positioner(float x, float y, bool is_movement) : is_movement{is_movement} {
       pos = ImVec2(x, y);
   }

   const void positioner::render_visible() {

       if(is_movement) {
           ImVec2 mv = ImGui::GetCursorPos();
           mv.x += pos.x;
           mv.y += pos.y;
           ImGui::SetCursorPos(mv);

       } else {
           if(pos.x < 0 && pos.y >= 0) {
               ImGui::SetCursorPosY(pos.y);
           } else if(pos.x >= 0 && pos.y < 0) {
               ImGui::SetCursorPosX(pos.x);
           } else {
               ImGui::SetCursorPos(pos);
           }
       }
   }
}