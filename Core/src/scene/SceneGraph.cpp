#include <pch.hpp>
#include "SceneGraph.hpp"

namespace gage::scene
{
    SceneGraph::SceneGraph()
    {
        // Create root node
        id = 0;
        auto node = std::make_unique<Node>(*this, id);
        node->name = "Root";
        nodes.push_back(std::move(node));
        id++;
    }
    SceneGraph::~SceneGraph()
    {
    }

    void SceneGraph::import_scene(const std::string& file_path, ImportMode mode)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
        loader.SetImageWriter(tinygltf::WriteImageData, nullptr);
    }

    // Every node created will be the child of the root node
    Node *SceneGraph::create_node()
    {
        auto node = std::make_unique<Node>(*this, id);
        auto node_ptr = node.get();

        auto root_node = nodes.at(0).get();
        node_ptr->parent = root_node;
        root_node->children.push_back(node_ptr);
        

        nodes.push_back(std::move(node));
        id++;


        return node_ptr;
    }

    void SceneGraph::make_parent(Node* parent, Node* child)
    {
        //Make sure to delete current child in the prev child's parent
        auto child_parent = child->parent;
        // get rid of children
        child_parent->children.erase(std::remove_if(child_parent->children.begin(), child_parent->children.end(), 
                            [&child](const Node* node) { return node == child; }));

        parent->children.push_back(child);
        child->parent = parent;
    }

    Node *SceneGraph::find_node(uint64_t id)
    {
        for (const auto &node : nodes)
        {
            if (node->id == id)
                return node.get();
        }

        return nullptr;
    }

    uint64_t SceneGraph::find_node_index(uint64_t id)
    {
        uint64_t index = 0;
        for (const auto &node : nodes)
        {
            if (node->id == id)
            {
                return index;
            }
            index++;
        }

        return 0;
    }

    const std::vector<std::unique_ptr<Node>> &SceneGraph::get_nodes() const
    {
        return nodes;
    }
}