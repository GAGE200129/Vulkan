#include <pch.hpp>
#include "ModelNode.hpp"

#include <tiny_gltf.h>

namespace gage::scene::data
{
    ModelNode::ModelNode(const tinygltf::Node &node, uint32_t node_index)
    {
        name = node.name;
        bone_id = node_index;

        if (node.mesh > -1)
        {
            has_mesh = true;
            mesh_index = node.mesh;
        }

        if (node.skin > -1)
        {
            has_skin = true;
            skin_index = node.skin;
        }

        assert(node.matrix.size() == 0);

        if (node.translation.size() != 0)
        {
            position.x = node.translation.at(0);
            position.y = node.translation.at(1);
            position.z = node.translation.at(2);
        }

        if (node.scale.size() != 0)
        {
            scale.x = node.scale.at(0);
            scale.y = node.scale.at(1);
            scale.z = node.scale.at(2);
        }

        if (node.rotation.size() != 0)
        {
            rotation.x = node.rotation.at(0);
            rotation.y = node.rotation.at(1);
            rotation.z = node.rotation.at(2);
            rotation.w = node.rotation.at(3);
        }

        children.reserve(node.children.size());
        for (const auto &child : node.children)
        {
            children.push_back(child);
        }
    }
}