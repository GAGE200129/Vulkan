#include <pch.hpp>
#include "Animator.hpp"

#include "../data/Model.hpp"
#include "../systems/Animation.hpp"
#include "../Node.hpp"

#include <imgui/imgui.h>

#include "MeshRenderer.hpp"

namespace gage::scene::components
{
    Animator::Animator(SceneGraph &scene, Node &node, const data::Model &model, const std::vector<data::ModelAnimation> &model_animations) : IComponent(scene, node),
                                                                                                                                             model(model),
                                                                                                                                             model_animations(model_animations)

    {
    }

    void Animator::render_imgui()
    {

        if (ImGui::BeginListBox("Animations"))
        {

            for (const auto &animation : this->model_animations)
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