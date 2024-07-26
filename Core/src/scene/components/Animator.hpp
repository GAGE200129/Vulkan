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
        Animator(SceneGraph &scene, Node &node, const data::Model &model, const std::vector<data::ModelAnimation> &model_animations);

        void init() final;
        void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) final;
        void late_update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) final;
        inline void render_depth(VkCommandBuffer, VkPipelineLayout) final {}
        inline void render_geometry(VkCommandBuffer, VkPipelineLayout) final {}
        inline void shutdown() final {}

        inline void render_imgui() final {};
        inline const char* get_name() const final { return "Animator"; };


        void set_current_animation(const std::string &name);

    private:
        const data::Model &model;
        const std::vector<data::ModelAnimation> &model_animations;

        std::vector<MeshRenderer*> p_mesh_renderers{};
        double current_time{0.0};
        std::map<uint32_t, Node *> bone_id_to_joint_map{};
        std::map<uint32_t, Node *> skeleton_id_to_joint_map{};
        const data::ModelAnimation *current_animation{nullptr};
    };
}