#include <pch.hpp>
#include "MeshRenderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"




namespace gage::scene::components
{
    MeshRenderer::MeshRenderer(SceneGraph &scene,
         Node &node, const gfx::Graphics &gfx,
         const data::Model &model,
         const data::ModelMesh &model_mesh,
         const std::vector<uint32_t>* joints) : 
        IComponent(scene, node),
        gfx(gfx),
        model(model),
        model_mesh(model_mesh),
        joints(joints)

    {}


    nlohmann::json MeshRenderer::to_json() const
    {
        return { {"type", get_name()}, {"model", model.name} };
    }


    MeshRenderer::AnimationBuffer& MeshRenderer::get_animation_buffer()
    {
        return animation_buffer_data;
    }

}