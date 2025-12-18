#pragma once
#include "../widgets.h"

namespace grey::x {
    struct graph_node {
        int id;
        ImVec2 pos;
        grey::widgets::rgb_colour fill_colour{0.2f, 0.6f, 0.9f, 1.0f};
        grey::widgets::rgb_colour text_colour{1.0f, 1.0f, 1.0f, 1.0f};
        grey::widgets::rgb_colour border_colour{0.3f, 0.7f, 1.0f, 1.0f};
        float radius{25.0f};
    };

    struct graph_edge {
        int from_node_id;
        int to_node_id;
        grey::widgets::rgb_colour colour{0.8f, 0.8f, 0.8f, 1.0f};
    };

    class graph {
    public:
        void render();

        void add_node(int id);
        void add_edge(int from_node_id, int to_node_id);

        /**
         * @brief Positions all nodes in a circular layout within the given canvas size.
         * @param canvas_width 
         * @param canvas_height 
         */
        void layout_circle();

        /**
         * @brief Performs a Fruchterman-Reingold force-directed graph layout algorithm. This call computes single loop iteration.
         * @return True if the layout is stable, false otherwise;
         */
        bool layout_fruchterman_reingold(float temperature = 2.0f, float attraction = 1.5f, float repulsion = 0.5f);

        const std::vector<graph_node>& get_nodes() const;



    private:
        std::vector<graph_node> nodes;
        std::vector<graph_edge> edges;
    };
}