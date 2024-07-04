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
               int32_t mesh) : 
            gfx(gfx),
            model(model),
            position(position),
            rotation(rotation),
            scale(scale),
            children(std::move(children)),
            mesh(mesh)
    {
    }

    void Node::draw(VkCommandBuffer cmd) const
    {
        for(const auto& child : children)
        {
            model.nodes.at(child).draw(cmd);
        }


        if(mesh < 0)
            return;
        // Build push constant transform
        glm::mat4x4 transform = glm::scale(glm::mat4x4(1.0f), scale);
        transform *= glm::mat4x4(rotation);
        transform = glm::translate(transform, position);

        gfx.get_default_pipeline().set_push_constant(cmd, transform);
        model.meshes.at(mesh).draw(cmd);


    }

}