#include <pch.hpp>
#include "Animator.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <set>

#include "MeshRenderer.hpp"

namespace gage::scene::components
{
    Animator::Animator(SceneGraph &scene, Node &node, const data::Model &model, const std::vector<data::ModelAnimation> &model_animations) : IComponent(scene, node),
                                                                                                                                             model(model),
                                                                                                                                             model_animations(model_animations)

    {
    }

}