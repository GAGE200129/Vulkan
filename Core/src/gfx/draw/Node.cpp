#include <pch.hpp>
#include "Node.hpp"

#include "Model.hpp"
#include "Mesh.hpp"
#include "../Graphics.hpp"

namespace gage::gfx::draw
{
    Node::Node(Graphics &gfx, const Model &model,
               const glm::vec3 &position,
               const glm::quat &rotation,
               const glm::vec3 &scale,
               std::vector<uint32_t> children,
               int32_t mesh) : gfx(gfx),
                               model(model),
                               position(position),
                               rotation(rotation),
                               scale(scale),
                               children(std::move(children)),
                               mesh(mesh)
    {
    }

    void Node::draw(VkCommandBuffer cmd, VkPipelineLayout layout, const data::Frustum &frustum) const
    {
        for (const auto &child : children)
        {
            model.nodes.at(child)->draw(cmd, layout, frustum);
        }

        if (mesh < 0)
            return;
        // Build push constant transform
        glm::mat4x4 transform = glm::scale(glm::mat4x4(1.0f), scale);
        transform *= glm::mat4x4(rotation);
        transform = glm::translate(transform, position);
        const auto &mesh_instance = model.meshes.at(mesh);

        auto isOnOrForwardPlane = [](const data::Plane &plane, const glm::vec3 &center, float radius)
        {
            float signed_distance = glm::dot(plane.normal, center - plane.point);
            return signed_distance > -radius;
        };

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, translation, skew, perspective);
 

        // Check Firstly the result that have the most chance
        // to faillure to avoid to call all functions.
        
        float radius = mesh_instance->get_cull_radius();
        const float max_scale = std::max({scale.x, scale.y, scale.z});
        radius *= max_scale * 0.5f;

        if (!frustum.enabled || (glm::length2(frustum.near.point - translation) < glm::pow(radius, 2)) ||
            (isOnOrForwardPlane(frustum.left, translation, radius) &&
            isOnOrForwardPlane(frustum.right, translation, radius) &&
            isOnOrForwardPlane(frustum.far, translation, radius) &&
            isOnOrForwardPlane(frustum.near, translation, radius) &&
            isOnOrForwardPlane(frustum.top, translation, radius) && 
            isOnOrForwardPlane(frustum.bottom, translation, radius)))
        {
            mesh_instance->draw(cmd, layout, transform);
        }
    }

}