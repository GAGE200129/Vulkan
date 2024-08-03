#include <pch.hpp>
#include "MeshRenderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"




namespace gage::scene::components
{
    MeshRenderer::MeshRenderer(SceneGraph &scene, Node &node, gfx::Graphics &gfx, const data::Model &model, const data::ModelMesh &model_mesh, const data::ModelSkin* model_skin) : 
        IComponent(scene, node),
        gfx(gfx),
        model(model),
        model_mesh(model_mesh),
        model_skin(model_skin)

    {}


    nlohmann::json MeshRenderer::to_json() const
    {
        return { {"type", get_name()}, {"model", model.name} };
    }


    MeshRenderer::AnimationBuffer& MeshRenderer::get_animation_buffer()
    {
        return animation_buffer_data;
    }

    const data::ModelSkin* MeshRenderer::get_skin()
    {
        return model_skin;
    }
}