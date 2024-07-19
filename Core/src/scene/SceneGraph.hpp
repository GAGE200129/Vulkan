#pragma once

#include "Node.hpp"

#include <vector>
#include <cstdint>
#include <memory>

namespace gage::scene
{
    class SceneGraph
    {
    public:
        enum class ImportMode
        {
            Binary,
            ASCII
        };
    public:
        SceneGraph();
        ~SceneGraph();

        Node* create_node();
        Node* find_node(uint64_t id);
        uint64_t find_node_index(uint64_t id);

        void make_parent(Node* parent, Node* child);

        void import_scene(const std::string& file_path, ImportMode mode);


        const std::vector<std::unique_ptr<Node>>& get_nodes() const;
    private:
        uint64_t id{0};
        std::vector<std::unique_ptr<Node>> nodes{};
        uint32_t root_node{};
    };
}