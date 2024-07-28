#pragma once

#include "IComponent.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>

namespace gage::scene::data
{
    class Model;
    class ModelMesh;
    class ModelSkin;
}

namespace gage::scene::systems
{
    class Renderer;
}


namespace gage::scene::components
{
    class MeshRenderer final : public IComponent
    {
        friend class systems::Renderer;
    public:
        struct AnimationBuffer
        {
            glm::mat4x4 bone_matrices[100]{};
            uint32_t enabled{}; 
        };
    public:
        MeshRenderer(SceneGraph& scene, Node& node, gfx::Graphics& gfx, const data::Model& model, const data::ModelMesh& model_mesh, const data::ModelSkin* model_skin);

        inline void render_imgui() final {};
        inline const char* get_name() const final { return "MeshRenderer"; };

        AnimationBuffer* get_animation_buffer();
        const data::ModelSkin* get_skin();
    private:
        
        gfx::Graphics& gfx;
        const data::Model& model;
        const data::ModelMesh& model_mesh;
        const data::ModelSkin* model_skin{};
        std::unique_ptr<gfx::data::CPUBuffer> animation_buffers[gfx::Graphics::FRAMES_IN_FLIGHT]{};
        VkDescriptorSet animation_descs[gfx::Graphics::FRAMES_IN_FLIGHT]{};
    };
}