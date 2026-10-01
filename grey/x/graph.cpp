#include "graph.h"
#include <cmath>
#include <unordered_map>

namespace w = grey::widgets;

namespace grey::x {
    void graph::render() {
        ImVec2 pos = w::cur_get();
        float width = w::avail_x();
        float height = w::avail_y();
        auto& style = ImGui::GetStyle();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + width, pos.y + height), IM_COL32(25, 25, 25, 255));

        ImVec2 mouse_pos = ImGui::GetMousePos();
        bool mouse_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
        bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool mouse_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        if (mouse_released) {
            dragging_node_id = -1;
        }

        if (mouse_clicked && dragging_node_id == -1) {
            for (auto& node : nodes) {
                ImVec2 node_center = ImVec2(pos.x + node.pos.x + 50, pos.y + node.pos.y + 25);
                float dx = mouse_pos.x - node_center.x;
                float dy = mouse_pos.y - node_center.y;
                float dist_sq = dx * dx + dy * dy;
                
                if (dist_sq <= node.radius * node.radius) {
                    dragging_node_id = node.id;
                    drag_offset = ImVec2(node_center.x - mouse_pos.x, node_center.y - mouse_pos.y);
                    break;
                }
            }
        }

        if (mouse_down && dragging_node_id != -1) {
            auto it = std::find_if(nodes.begin(), nodes.end(),
                [this](const graph_node& n) { return n.id == dragging_node_id; });
            
            if (it != nodes.end()) {
                ImVec2 new_center = ImVec2(mouse_pos.x + drag_offset.x, mouse_pos.y + drag_offset.y);
                it->pos.x = new_center.x - pos.x - 50;
                it->pos.y = new_center.y - pos.y - 25;
            }
        }

        // draw edges first, so nodes appear on top
        for(const graph_edge& edge : edges) {
            auto it_from = std::find_if(nodes.begin(), nodes.end(),
                [&edge](const graph_node& n) { return n.id == edge.from_node_id; });
            auto it_to = std::find_if(nodes.begin(), nodes.end(),
                [&edge](const graph_node& n) { return n.id == edge.to_node_id; });
            if(it_from != nodes.end() && it_to != nodes.end()) {
                ImVec2 from_pos = ImVec2(pos.x + it_from->pos.x + 50, pos.y + it_from->pos.y + 25);
                ImVec2 to_pos = ImVec2(pos.x + it_to->pos.x + 50, pos.y + it_to->pos.y + 25);
                dl->AddLine(from_pos, to_pos, edge.colour, 2.0f);
            }
        }

        // draw nodes
        for(const graph_node& node : nodes) {
            ImVec2 node_center = ImVec2(pos.x + node.pos.x + 50, pos.y + node.pos.y + 25);
            
            bool is_dragging = (node.id == dragging_node_id);
            float visual_radius = is_dragging ? node.radius * 1.1f : node.radius;
            
            dl->AddCircleFilled(node_center, visual_radius, node.fill_colour);
            dl->AddCircle(node_center, visual_radius, node.border_colour, 0, 2.0f);
            
            ImVec2 text_size = ImGui::CalcTextSize(std::to_string(node.id).c_str());
            ImVec2 text_pos = ImVec2(node_center.x - text_size.x * 0.5f, node_center.y - text_size.y * 0.5f);
            dl->AddText(text_pos, node.text_colour, std::to_string(node.id).c_str());
        }
    }

    graph_node& graph::add_node(int id) {
        graph_node n{id};
        //n.pos.x = w::avail_x() / 2;
        //n.pos.y = w::avail_y() / 2;
        nodes.push_back(n);
        return nodes.back();
    }

    void graph::add_edge(int from_node_id, int to_node_id) {
        edges.emplace_back(from_node_id, to_node_id);
    }

    void graph::layout_circle() {

        float canvas_width = w::avail_x();
        float canvas_height = w::avail_y();

        // position nodes in a circle
        size_t n = nodes.size();
        float center_x = canvas_width / 2.0f;
        float center_y = canvas_height / 2.0f;
        float radius = std::min(canvas_width, canvas_height) / 2.5f;
        for (size_t i = 0; i < n; ++i) {
            float angle = (2.0f * 3.14159265f * i) / n;
            nodes[i].pos.x = center_x + radius * std::cos(angle);
            nodes[i].pos.y = center_y + radius * std::sin(angle);
        }
    }

    bool graph::layout_fruchterman_reingold(float temperature, float attraction, float repulsion) {

        float canvas_width = w::avail_x();
        float canvas_height = w::avail_y();

        if (nodes.empty()) return true;

        const float area = canvas_width * canvas_height;
        const float k = std::sqrt(area / nodes.size()) * 0.5f;
        const float c_repulsion = k * k * repulsion;

        std::unordered_map<int, ImVec2> displacement;
        for (auto& node : nodes) {
            displacement[node.id] = ImVec2(0.0f, 0.0f);
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            for (size_t j = i + 1; j < nodes.size(); ++j) {
                ImVec2 delta = ImVec2(
                    nodes[i].pos.x - nodes[j].pos.x,
                    nodes[i].pos.y - nodes[j].pos.y
                );
                
                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                if (distance < 0.01f) distance = 0.01f;

                float repulsion_force = c_repulsion / distance;
                ImVec2 force = ImVec2(
                    (delta.x / distance) * repulsion_force,
                    (delta.y / distance) * repulsion_force
                );

                displacement[nodes[i].id].x += force.x;
                displacement[nodes[i].id].y += force.y;
                displacement[nodes[j].id].x -= force.x;
                displacement[nodes[j].id].y -= force.y;
            }
        }

        for (const auto& edge : edges) {
            auto it_from = std::find_if(nodes.begin(), nodes.end(),
                [&edge](const graph_node& n) { return n.id == edge.from_node_id; });
            auto it_to = std::find_if(nodes.begin(), nodes.end(),
                [&edge](const graph_node& n) { return n.id == edge.to_node_id; });

            if (it_from != nodes.end() && it_to != nodes.end()) {
                ImVec2 delta = ImVec2(
                    it_to->pos.x - it_from->pos.x,
                    it_to->pos.y - it_from->pos.y
                );

                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                if (distance < 0.01f) distance = 0.01f;

                float attraction_force = attraction * (distance * distance) / k;
                ImVec2 force = ImVec2(
                    (delta.x / distance) * attraction_force,
                    (delta.y / distance) * attraction_force
                );

                displacement[it_from->id].x += force.x;
                displacement[it_from->id].y += force.y;
                displacement[it_to->id].x -= force.x;
                displacement[it_to->id].y -= force.y;
            }
        }

        bool is_stable = true;
        const float stability_threshold = 0.1f;

        for (auto& node : nodes) {
            ImVec2& disp = displacement[node.id];
            float disp_length = std::sqrt(disp.x * disp.x + disp.y * disp.y);
            
            if (disp_length > stability_threshold) {
                is_stable = false;
            }

            if (disp_length > 0.01f) {
                float capped = std::min(disp_length, temperature);
                node.pos.x += (disp.x / disp_length) * capped;
                node.pos.y += (disp.y / disp_length) * capped;

                node.pos.x = std::max(50.0f, std::min(node.pos.x, canvas_width - 150.0f));
                node.pos.y = std::max(25.0f, std::min(node.pos.y, canvas_height - 75.0f));
            }
        }

        return is_stable;
    }

    const std::vector<graph_node>& graph::get_nodes() const {
        return nodes;
    }
}