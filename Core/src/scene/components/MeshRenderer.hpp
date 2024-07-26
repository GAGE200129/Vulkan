#pragma once

#include "IComponent.hpp"

#include <Core/src/gfx/data/CPUBuffer.hpp>

namespace gage::scene::data
{
    class Model;
    class ModelMesh;
    class ModelSkin;
}


namespace gage::scene::components
{
    class MeshRenderer final : public IComponent
    {
    public:
        struct AnimationBuffer
        {
            glm::mat4x4 bone_matrices[100]{};
            uint32_t enabled{}; 
        };
    public:
        MeshRenderer(SceneGraph& scene, Node& node, gfx::Graphics& gfx, const data::Model& model, const data::ModelMesh& model_mesh, const data::ModelSkin* model_skin);

        inline void init() final {}
        inline void update(float, const hid::Keyboard&, const hid::Mouse&) final {}
        inline void late_update(float, const hid::Keyboard&, const hid::Mouse&) final {}
        void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void shutdown() final;

        inline void render_imgui() final {};
        inline const char* get_name() const final { return "MeshRenderer"; };

        AnimationBuffer* get_animation_buffer();
        const data::ModelSkin* get_skin();
    private:
        
        gfx::Graphics& gfx;
        const data::Model& model;
        const data::ModelMesh& model_mesh;
        const data::ModelSkin* model_skin{};
        gfx::data::CPUBuffer bone_matrices_buffer;
        VkDescriptorSet animation_desc{};
    };
}