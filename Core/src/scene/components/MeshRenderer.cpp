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


    MeshRenderer::AnimationBuffer* MeshRenderer::get_animation_buffer()
    {
        return (AnimationBuffer*)animation_buffers[gfx.get_current_frame_index()]->get_mapped();
    }

    const data::ModelSkin* MeshRenderer::get_skin()
    {
        return model_skin;
    }
}