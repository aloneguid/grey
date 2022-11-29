#include "components.h"
#include "imgui.h"
#include "imgui_internal.h"
//#include "implot.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include <cstdarg>
//#include "3rdparty/imnodes/imnodes.h"
#include <map>
#include <iostream>
#include <algorithm>

using namespace std;

namespace grey
{
   const size_t MAX_TABLE_COLUMNS = 64;

   const ImVec4 emphasis_primary_colour = (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f);
   const ImVec4 emphasis_primary_colour_hovered = (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.7f);
   const ImVec4 emphasis_primary_colour_active = (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.8f);

   const ImVec4 emphasis_error_colour = (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f);
   const ImVec4 emphasis_error_colour_hovered = (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.7f);
   const ImVec4 emphasis_error_colour_active = (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.8f);

   const ImVec4 emphasis_warning_colour = (ImVec4)ImColor::HSV(7 / 7.0f, 0.6f, 0.6f);
   const ImVec4 emphasis_warning_colour_hovered = (ImVec4)ImColor::HSV(7 / 7.0f, 0.6f, 0.7f);
   const ImVec4 emphasis_warning_colour_active = (ImVec4)ImColor::HSV(7 / 7.0f, 0.6f, 0.8f);


   void set_colours(emphasis em, ImVec4& normal, ImVec4& hovered, ImVec4& active)
   {
      switch (em)
      {
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

   int generate_int_id()
   {
      return incrementing_id++;
   }

   static string generate_id(const string& prefix = "")
   {
      return prefix + std::to_string(incrementing_id++);
   }

   const void component::render() {
       if (on_frame) {
           on_frame(*this);
       }

       if (is_visible != is_visible_prev_frame) {
           is_visible_prev_frame = is_visible;

           if (on_visibility_changed) {
               on_visibility_changed(*this);
           }
       }

       if (!is_visible) return;

       if (width != 0)
           ImGui::PushItemWidth(width);

       render_visible();

       if (width != 0)
           ImGui::PopItemWidth();

       if (on_hovered && ImGui::IsItemHovered()) {
           on_hovered(*this);
       }

       if (!tooltip.empty() && ImGui::IsItemHovered()) {
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

   const void common_component::render_visible()
   {
      switch (c)
      {
      case 1:
         ImGui::SameLine();
         break;
      case 2:
         ImGui::Spacing();
         break;
      case 3:
         ImGui::Separator();
         break;

      }
   }

   const void container::render()
   {
      if (!managed_children.empty())
      {
         for (const auto& mc : managed_children)
         {
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

   void container::assign_child(std::shared_ptr<component> child)
   {
      owned_children.push_back(child);
      child->parent = this;
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


   std::shared_ptr<grey::label> container::make_label(const string& text)
   {
      auto r = make_shared<label>(text);
      assign_child(r);
      return r;
   }

   std::shared_ptr<label> container::make_label(string* text)
   {
      auto r = make_shared<label>(text);
      assign_child(r);
      return r;
   }

   std::shared_ptr<menu_bar> container::make_menu_bar()
   {
      auto r = make_shared<menu_bar>();
      assign_child(r);
      return r;
   }

   std::shared_ptr<listbox> container::make_listbox(const string& label)
   {
      auto r = make_shared<listbox>(label);
      assign_child(r);
      return r;
   }

   std::shared_ptr<input> container::make_input(const string& label, string* value)
   {
      auto r = make_shared<input>(label, value);
      assign_child(r);
      return r;
   }

   std::shared_ptr<input_int> container::make_input_int(const string& label, int* value)
   {
      auto r = make_shared<input_int>(label, value);
      assign_child(r);
      return r;
   }

   std::shared_ptr<checkbox> container::make_checkbox(const string& label, bool* value)
   {
      auto r = make_shared<checkbox>(label, value);
      assign_child(r);
      return r;
   }

   std::shared_ptr<grey::tree> container::make_tree()
   {
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

   std::shared_ptr<plot> container::make_plot(const string& title, std::vector<float>& values)
   {
      auto r = make_shared<plot>(title, values);
      assign_child(r);
      return r;
   }

   std::shared_ptr<status_bar> container::make_status_bar()
   {
      auto r = make_shared<status_bar>(tmgr);
      assign_child(r);
      return r;
   }

   std::shared_ptr<accordion> container::make_accordion(const string& label)
   {
      auto r = make_shared<accordion>(tmgr, label, false);
      assign_child(r);
      return r;
   }

   std::shared_ptr<child> container::make_child_window(size_t width, size_t height, bool horizonal_scroll)
   {
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

   void container::same_line()
   {
      assign_child(make_shared<common_component>(1));
   }

   void container::spacer()
   {
      assign_child(make_shared<common_component>(2));
   }

   void container::separator()
   {
      assign_child(make_shared<common_component>(3));
   }

   std::shared_ptr<imgui_raw> container::make_raw_imgui()
   {
      auto r = make_shared<imgui_raw>();
      assign_child(r);
      return r;
   }

   const void container::render_children()
   {
      if (!owned_children.empty())
      {
         for (auto& child : owned_children)
         {
            child->render();
         }
      }
   }

#ifdef _DEBUG
   const void demo::render_visible() {
       ImGui::ShowDemoWindow();
   }
#endif

   grey::child::child(grey_context& mgr, size_t width, size_t height, bool horizonal_scroll)
       : container{mgr}, size{width, height} {
       if(horizonal_scroll)
           flags |= ImGuiWindowFlags_HorizontalScrollbar;
   }

   const void child::render_visible() {
       //ImGui::PushStyleColor(ImGuiCol_Border, )

       if(em != emphasis::none) {
           ImGui::PushStyleColor(ImGuiCol_Border, em_normal);
       }

       //ImVec2 sz2 = ImGui::GetWindowSize();
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

       ImGui::EndGroup();

       // item size can be used:
       // ImVec2 size = ImGui::GetItemRectSize();
   }

   grey::window::window(grey_context& mgr, string title,
                        bool is_maximized, bool can_close, bool is_dockspace)
   :
      container{ mgr }, title{ title },
      is_maximized{ is_maximized },
      can_close{ can_close },
      is_dockspace{ is_dockspace }
   {
      if (is_maximized)
      {
         flags = (
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            //ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize
            );

         // need to call PopStyleVar elsewhere
         //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
      }
      else
      {
         flags = 0;
         //flags |= ImGuiWindowFlags_NoDocking;
      }

      if (has_menu_space) flags |= ImGuiWindowFlags_MenuBar;
   }

   const void grey::status_bar::render_visible()
   {
      // source: https://github.com/ocornut/imgui/issues/3518#issuecomment-807398290
      //ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
      float height = ImGui::GetFrameHeight();
      if (ImGui::BeginViewportSideBar("##MainStatusBar", /*viewport*/nullptr, ImGuiDir_Down, height, flags))
      {
         if (ImGui::BeginMenuBar())
         {
            //ImGui::Text("Happy status bar");

            render_children();

            ImGui::EndMenuBar();
         }
      }

      ImGui::End(); // status bar, should be outside of BeginViewportSideBar just like for a normal window
   }

   accordion::accordion(grey_context& mgr, const string& label, bool is_closeable) : container{ mgr }, is_closeable{ is_closeable }
   {
      this->label = sys_label(label);
   }

   const void grey::accordion::render_visible()
   {
      if (is_closeable ? ImGui::CollapsingHeader(label.c_str(), &is_open) : ImGui::CollapsingHeader(label.c_str()))
      {
         render_children();
      }
   }

   void window::close() {
       this->user_is_open = false;
   }

   const void window::render_visible() {
       //if (ImGui::BeginMainMenuBar())
       //{
       //   this->owned_children[0]->render();

       //   ImGui::EndMainMenuBar();
       //}


       if (!is_open) return;

       ImGuiWindowFlags rflags = flags;
       if (has_menu_space) rflags |= ImGuiWindowFlags_MenuBar;

       if (is_maximized) {
           auto& io = ImGui::GetIO();
           ImGui::SetNextWindowPos(ImVec2(0, 0));
           ImGui::SetNextWindowSize(io.DisplaySize);

           //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
       }

       // the rest

       bool began = ImGui::Begin(title.c_str(), can_close ? &is_open : nullptr, rflags);

       if (is_maximized) {
           // ImGuiStyleVar_FramePadding
           //ImGui::PopStyleVar();
       }

       if (is_dockspace) {
           // useful docking links:
           // - https://github.com/ocornut/imgui/issues/4430
           // - https://gist.github.com/moebiussurfing/d7e6ec46a44985dd557d7678ddfeda99
           if (first_time) {
               first_time = false;

               dockspace_id = ImGui::GetID("MainDockspace");
           }

           ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
       }

       if (began) {
           if (!user_is_open) {
               is_open = false;
           }
           if (was_open != is_open) {
               if (on_open_changed) {
                   on_open_changed(is_open);
               }
               was_open = is_open;
           }

           render_children();
       }

       // for the window, End must be called regardless
       ImGui::End();
   }

   button::button(const string& label, bool is_small, emphasis e) : is_small{ is_small }
   {
      set_label(label);
      set_emphasis(e);
   }

   void button::set_label(const string& label)
   {
      this->label = sys_label(label);
   }

   const void button::render_visible()
   {
      bool disabled = !is_enabled;  // cache value as it might change after callback

      if (disabled)
      {
         ImGui::BeginDisabled(true);
      }
      
      if(em != emphasis::none)
      {
         ImGui::PushStyleColor(ImGuiCol_Button, em_normal);
         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, em_hovered);
         ImGui::PushStyleColor(ImGuiCol_ButtonActive, em_active);
      }

      if (is_small)
      {
         if (ImGui::SmallButton(label.c_str()) && on_pressed)
         {
            on_pressed(*this);
         }
      }
      else
      {
         if (ImGui::Button(label.c_str()) && on_pressed)
         {
            on_pressed(*this);
         }
      }

      if (disabled) ImGui::EndDisabled();
      if (em != emphasis::none) ImGui::PopStyleColor(3);
   }

   const void grey::toggle::render_visible()
   {
      if (*value)
      {
         if (!label_off.empty())
         {
            ImGui::Text(label_off.c_str());
            ImGui::SameLine();
         }
      }
      else
      {
         if (!label_on.empty())
         {
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
      if (ImGui::IsItemClicked())
      {
         *value = !*value;
         if (on_toggled) on_toggled(*value);
      }

      ImGuiContext& gg = *GImGui;
      float ANIM_SPEED = 0.085f;
      if (gg.LastActiveId == gg.CurrentWindow->GetID(id.c_str()))// && g.LastActiveIdTimer < ANIM_SPEED)
         float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
      if (ImGui::IsItemHovered())
         draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*value ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
      else
         draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*value ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
      draw_list->AddCircleFilled(ImVec2(p.x + radius + (*value ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
   }

   string label::get_value()
   {
      if (value_ptr)
         return *value_ptr;

      return value;
   }

   void label::set_value(const string& value)
   {
      // only allow changing it if we own it!
      if (!value_ptr)
         this->value = value;
   }

   const void label::render_visible()
   {
      if (text_wrap_pos > 0)
         ImGui::PushTextWrapPos(text_wrap_pos);

      const char* buf = value_ptr == nullptr ? value.c_str() : (*value_ptr).c_str();

      if (is_enabled)
      {
         if (em != emphasis::none)
         {
            ImGui::TextColored(em_normal, buf);
         }
         else
         {
            ImGui::Text(buf);
         }
      }
      else
      {
         ImGui::TextDisabled("%s", buf);
      }

      if (text_wrap_pos > 0)
         ImGui::PopTextWrapPos();
   }

   selectable::selectable(const std::string& value)
      : value{ sys_label(value) }
   {

   }

   const void selectable::render_visible()
   {
      // todo: flag can be set to appear as disabled
      //ImGuiSelectableFlags rflags = flags;

      bool align = alignment.x > 0 || alignment.y > 0;

      if (align)
         ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);

      if (ImGui::Selectable(value.c_str(), &is_selected, flags, size))
      {
         if (on_selected)
         {
            on_selected(*this);
         }
      }

      if (align)
         ImGui::PopStyleVar();
   }

   shared_ptr<menu_item> menu_item::add(const std::string& id, const std::string& label) {
       auto mi = make_shared<menu_item>(id, label);
       children.push_back(mi);
       return mi;
   }

   const void menu_bar::render_visible()
   {
      bool rendered = is_main_menu ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();

      if (rendered)
      {
         for (auto mi : root->children)
         {
            render(mi);
         }

         if (is_main_menu)
            ImGui::EndMainMenuBar();
         else
            ImGui::EndMenuBar();
      }
   }

   void menu_bar::render(shared_ptr<menu_item> mi)
   {
      if (mi->children.empty())
      {
         if (ImGui::MenuItem(mi->label.c_str(),
            mi->shortcut_text.empty() ? nullptr : mi->shortcut_text.c_str(),
            mi->is_selected, mi->is_enabled) && clicked && !mi->id.empty())
         {
            clicked(*mi);
         }
      }
      else
      {
         if (ImGui::BeginMenu(mi->label.c_str()))
         {
            for (auto imi : mi->children)
            {
               render(imi);
            }

            ImGui::EndMenu();
         }
      }

   }

   component::component(const string& id)
   {
      this->id = id.empty() ? generate_id() : id;
   }

   listbox::listbox(const string& title)
   {
      this->title = sys_label(title);
   }

   const void listbox::render_visible()
   {
      ImVec2 size = is_full_width
         ? ImVec2(-FLT_MIN, items_tall * ImGui::GetTextLineHeightWithSpacing())
         : ImVec2(0, items_tall * ImGui::GetTextLineHeightWithSpacing());

      // begin rendering
      bool ok;
      switch (mode)
      {
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
      default:
         ok = false;
         break;
      }

      // render items
      if (ok)
      {
         size_t si = 0;

         for (list_item& item : items)
         {
            bool is_selected = selected_index == si;

            if (ImGui::Selectable(item.text.c_str(), is_selected))
            {
               selected_index = si;
               if (on_selected)
               {
                  on_selected(si, item);
               }
            }

            if (is_selected)
               ImGui::SetItemDefaultFocus();

            if (!item.tooltip.empty() && ImGui::IsItemHovered())
            {
               ImGui::SetTooltip(item.tooltip.c_str());
            }

            si += 1;
         }

         // end rendering
         switch (mode)
         {
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
   bool tree_node::x_tree_node()
   {
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
      if (hovered || active)
         ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            ImColor(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

      // Icon, text
      ImGui::SameLine(x);
      //ImGui::ColorButton(x_opened ? ImColor(255, 0, 0) : ImColor(0, 255, 0));
      //ImGui::SameLine();
      ImGui::Text(label.c_str());
      ImGui::EndGroup();
      if (x_opened)
         ImGui::TreePush(label.c_str());
      return x_opened;
   }

   const void tree_node::render_visible()
   {
      ImGuiTreeNodeFlags flags = 
         ImGuiTreeNodeFlags_SpanAvailWidth;

      if (is_bold) flags |= ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
      if (is_expanded) flags |= ImGuiTreeNodeFlags_DefaultOpen;
      if (is_leaf) flags |= ImGuiTreeNodeFlags_Leaf;


      if(em != emphasis::none)
      {
         ImGui::PushStyleColor(ImGuiCol_Header, em_normal);
         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, em_hovered);
         ImGui::PushStyleColor(ImGuiCol_HeaderActive, em_active);
      }

      //ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 0, 0, 1));
      // use overload that allows passing node ID instead of just label, so node persists on rename
      if (ImGui::TreeNodeEx(id.c_str(), flags, label.c_str()))
      //if (x_tree_node())
      {
         render_children();

         for (auto child : nodes)
         {
            child->render();
         }

         ImGui::TreePop();
      }

      if(em != emphasis::none) ImGui::PopStyleColor(3);
   }

   std::shared_ptr<tree_node> tree_node::add_node(const string& label, bool is_expanded, bool is_leaf)
   {
      auto node = make_shared<tree_node>(tmgr, label, is_expanded, is_leaf);
      nodes.push_back(node);
      return node;
   }

   std::shared_ptr<tree_node> tree::add_node(const string& label, bool is_expanded, bool is_leaf)
   {
      auto node = make_shared<tree_node>(tmgr, label, is_expanded, is_leaf);
      nodes.push_back(node);
      return node;
   }

   void tree::remove_node(size_t index)
   {
      nodes.erase(nodes.begin() + index);
   }

   const void tree::render_visible()
   {
      for (auto node : nodes)
      {
         if (node)
         {
            node->render();
         }
      }
   }

   input::input(const string& label, string* value)
      : value { value }
   {
      //buffer.resize(buf_size);
      owns_mem = value == nullptr;
      if (owns_mem)
      {
         this->value = new string();
      }

      this->label = sys_label(label);
   }

   input::~input()
   {
      if (owns_mem)
      {
         delete value;
      }
   }

   const void input::render_visible()
   {
      bool is_error = is_in_error_state;
      if (is_error)
         ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.00f));

      ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
      if (!is_enabled) ImGui::BeginDisabled();

      if (lines <= 1)
      {
         if (ImGui::InputText(label.c_str(), value))
         {
            fire_changed();
         }
      }
      else
      {
         ImVec2 size;

         if (fill && parent)
         {
            size = ImGui::GetWindowSize();
            size.x = -FLT_MIN;
            size.y -= ImGui::GetTextLineHeight() * (3 + lines_bottom_padding);
         }
         else
         {
            size = ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * lines);
         }

         if (ImGui::InputTextMultiline(label.c_str(), value,
            size,
            flags))
         {
            fire_changed();
         }
      }
      if (!is_enabled) ImGui::EndDisabled();
      //ImGui::InputText(label.c_str(), &buffer[0], buf_size);

      if (is_error)
         ImGui::PopStyleColor();
   }

   void input::fire_changed()
   {
      if (on_value_changed)
      {
         on_value_changed(*value);
      }
   }

   input_int::input_int(const string& label, int* value) : value{ value }
   {
      owns_mem = value == nullptr;
      if (owns_mem)
      {
         this->value = new int();
      }

      this->label = sys_label(label);
   }

   input_int::~input_int()
   {
      if (owns_mem)
      {
         delete value;
      }
   }

   const void input_int::render_visible()
   {
      if (ImGui::InputInt(label.c_str(), value))
      {
         if (on_value_changed)
         {
            on_value_changed(*value);
         }
      }
   }

   /*const void plot::render_visible()
   {
      //int bar_data[5] = { 1, 2, 3, 4, 5 };
      //if (ImPlot::BeginPlot("My Plot"))
      //{
      //   ImPlot::PlotBars("Bars", bar_data, 5);

      //   ImPlot::EndPlot();
      //}

      //auto& io = ImGui::GetIO();
      //ImPlot::PlotLine()

      //ImPlot::ShowDemoWindow();

      float history = 10;
      ImPlot::SetNextPlotLimitsX(buffer.last_x - history, buffer.last_x, ImGuiCond_Always);
      ImPlot::SetNextPlotLimitsY(0, 80);
      if (ImPlot::BeginPlot(id.c_str(), "time", "FPS", ImVec2(-1, 0),
         ImPlotAxisFlags_NoTickLabels,
         ImPlotAxisFlags_NoTickLabels))
      {
         ImPlot::PlotLine("FPS",
            &buffer.Data[0].x, &buffer.Data[0].y, buffer.Data.size(), buffer.Offset, 2 * sizeof(float));

         ImPlot::EndPlot();
      }
   }

   void plot::add_point(float y)
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
      file_path_or_tag{ file_path },
      buffer{ buffer }, len{ len },
      mgr{ mgr },
      desired_width{ desired_width }, desired_height{ desired_height }
   {
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

   const void image::render_visible()
   {
      if (texture)
      {
         ImGui::Image(texture, ImVec2(desired_width, desired_height));
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

   const void slider::render_visible()
   {
      if (ImGui::SliderFloat(label.c_str(), value, min, max, format, flags))
      {
         value_changed = true;
      }

      if (value_changed && !ImGui::IsAnyMouseDown())
      {
         if (on_value_changed)
         {
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
       is_visible = false;
   }

   const void modal_popup::render() {
       // to open, call ImGui::OpenPopup(id.c_str()); _once_

       // to close, call CloseCurrentPopup _once_

       // do not fuck up this flag flipping logic, it's very fragile!!!!

       if(!sys_open) {  // check sys flip state - set to false by ImGui when popup closure is handled by imgui itself
           is_visible = false;
           was_visible = false;
           sys_open = true;
       }

       if(!was_visible && is_visible) {
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

   const void checkbox::render_visible()
   {
      if (is_highlighted)
      {
         ImGui::PushStyleColor(
            ImGuiCol_CheckMark,
            (ImVec4)ImColor::HSV(5.0f, 255.0f, 255.0f));
      }

      if (ImGui::Checkbox(text.c_str(), value))
      {
         if (on_value_changed)
            on_value_changed(*value);
      }

      if (is_highlighted)
      {
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
       flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
       if(tab_list_popup)
           flags |= ImGuiTabBarFlags_TabListPopupButton;
   }

   const void tabs::render_visible() {
       if (tab_headers.empty()) return;

       for (size_t i = 0; i < tab_headers.size(); i++) {
           if (ImGui::BeginTabBar(id.c_str(), flags)) {
               if (ImGui::BeginTabItem(tab_headers[i].c_str())) {
                   tab_containers[i]->render_visible();
                   ImGui::EndTabItem();
               }
               ImGui::EndTabBar();
           }
       }
   }

   void tabs::clear() {
       tab_headers.clear();
       tab_containers.clear();
   }

   std::shared_ptr<tab> tabs::make(const std::string& title) {
       auto c = make_shared<tab>(mgr);
       tab_headers.push_back(title);
       tab_containers.push_back(c);
       return c;
   }

   const void hex_editor::render_visible()
   {
      me.DrawContents((void*)(&data[0]), data.size());
   }

   const void plot::render_visible()
   {
      if (values.empty()) return;

      // autoscale
      float min = values[0];
      float max = values[0];
      for (float f : values)
      {
         min = std::min(min, f);
         max = std::max(max, f);
      }

      ImVec2 ws = ImGui::GetWindowSize();
      ImGuiStyle& style = ImGui::GetStyle();
      
      if (stick_to_bottom)
      {
         ImGui::SetCursorPosY(ws.y - height - style.ItemSpacing.y * 2);
      }

      ImVec2 size = ImVec2(width == -1
         ? (ws.x - style.ItemSpacing.x * 2)
         : width,
         height);

      ImGui::PlotHistogram(
         label.c_str(),
         &values[0], values.size(),
         0, NULL,
         min, max,
         size);
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
           if(pos.x < 0 && pos.y > 0) {
               ImGui::SetCursorPosY(pos.y);
           } else if(pos.x > 0 && pos.y < 0) {
               ImGui::SetCursorPosX(pos.x);
           } else {
               ImGui::SetCursorPos(pos);
           }
       }
   }
}