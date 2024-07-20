#pragma once

#include "IComponent.hpp"

namespace gage::scene::data
{
    class Model;
    class ModelMesh;
}

namespace gage::scene::components
{
    class MeshRenderer final : public IComponent
    {
    public:
        MeshRenderer(SceneGraph& scene, Node& node, const data::Model& model, const data::ModelMesh& model_mesh) :
            IComponent(scene, node),
            model(model),
            model_mesh(model_mesh)
        {}

        void init() final;
        void update(float delta) final;
        void render(gfx::Graphics& gfx, VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void shutdown() final;
    private:
        const data::Model& model;
        const data::ModelMesh& model_mesh;
    };
}