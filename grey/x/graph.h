#pragma once
#include "../widgets.h"

namespace grey::x {
    struct graph_node {
        int id;
        ImVec2 pos;
    };

    struct graph_edge {
        int from_node_id;
        int to_node_id;
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