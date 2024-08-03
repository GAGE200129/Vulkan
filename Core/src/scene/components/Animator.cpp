#include <pch.hpp>
#include "Animator.hpp"

#include "../data/Model.hpp"
#include "../systems/Animation.hpp"
#include "../Node.hpp"

#include <imgui/imgui.h>

#include "MeshRenderer.hpp"

namespace gage::scene::components
{
    Animator::Animator(SceneGraph &scene, Node &node, const data::Model &model) :
        IComponent(scene, node),
        model(model)

    {
    }

    nlohmann::json Animator::to_json() const
    {
        return { {"type", get_name()}, {"model", model.name} };
    }

    void Animator::render_imgui()
    {

        if (ImGui::BeginListBox("Animations"))
        {

            for (const auto &animation : this->model.animations)
            {
                if(ImGui::Selectable(animation.name.c_str(), current_animation == &animation))
                {
                    systems::Animation::set_animator_animation(this, animation.name);
                }
            }

            ImGui::EndListBox();
        }
    }

}