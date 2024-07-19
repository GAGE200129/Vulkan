#include <pch.hpp>
#include "Node.hpp"

namespace gage::scene
{
    Node::Node(SceneGraph& scene, uint64_t id) :
        scene(scene),
        id(id)
    {

    }
    Node::~Node()
    {

    }
    void Node::set_name(const std::string& name)
    {
        this->name = name;
    }

    uint64_t Node::get_id() const
    {
        return id;
    }

    const std::vector<Node*>& Node::get_children() const
    {
        return children;
    } 

    const std::string& Node::get_name() const
    {
        return name;
    }
}