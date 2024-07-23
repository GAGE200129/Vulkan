#pragma once

#include "IComponent.hpp"

#include <vector>
#include <memory>
#include <map>

namespace gage::scene::data
{
    class Model;
    class ModelAnimation;
}


namespace gage::scene::components
{
    class MeshRenderer;
    class Animator final : public IComponent
    {
    public:
        Animator(SceneGraph& scene, Node& node, const data::Model& model, const std::vector<data::ModelAnimation>& model_animations);

        void init() final;
        void update(float delta) final;
        void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void shutdown() final;

        void set_current_animation(const std::string& name);
    private:
        const data::Model& model;
        const std::vector<data::ModelAnimation>& model_animations;

        MeshRenderer* p_mesh_renderer{};
        double current_time{0.0};
        std::map<uint32_t, Node*> bone_id_to_joint_map{};
        std::map<uint32_t, Node*> skeleton_id_to_joint_map{};
        const data::ModelAnimation* current_animation{nullptr};
    };
}