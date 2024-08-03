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

namespace gage::scene::systems
{
    class Animation;
}

namespace gage::scene::components
{
    class MeshRenderer;
    class Animator final : public IComponent
    {
        friend class systems::Animation;
    public:
        Animator(SceneGraph &scene, Node &node, const data::Model &model);

        nlohmann::json to_json() const final;
        void render_imgui() final;
        inline const char* get_name() const final { return "Animator"; };


    private:
        const data::Model &model;

        std::vector<MeshRenderer*> p_mesh_renderers{};
        double current_time{0.0};
        std::map<uint32_t, Node *> bone_id_to_joint_map{};
        std::map<uint32_t, Node *> skeleton_id_to_joint_map{};
        const data::ModelAnimation *current_animation{nullptr};
    };
}