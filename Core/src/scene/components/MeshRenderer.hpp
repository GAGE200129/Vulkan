#pragma once

#include "IComponent.hpp"


namespace gage::gfx
{
    class Graphics;
}

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
    public:
        struct AnimationBuffer
        {
            glm::mat4x4 bone_matrices[100]{};
            uint32_t enabled{}; 
        };
    public:
        MeshRenderer(SceneGraph& scene, Node& node, const gfx::Graphics& gfx, const data::Model& model, const data::ModelMesh& model_mesh, const std::vector<uint32_t>* joints);

        nlohmann::json to_json() const final;

        inline void render_imgui() final {};
        inline const char* get_name() const final { return "MeshRenderer"; };

        AnimationBuffer& get_animation_buffer();
        const data::ModelSkin* get_skin();
    private:
        const gfx::Graphics& gfx;
    public:
        const data::Model& model;
        const data::ModelMesh& model_mesh;
        const std::vector<uint32_t>* joints{};
        AnimationBuffer animation_buffer_data{};
    };
}