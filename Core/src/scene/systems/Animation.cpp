#include <pch.hpp>
#include "Animation.hpp"

#include "../scene.hpp"
#include "../Node.hpp"
#include "../data/Model.hpp"

#include "../components/MeshRenderer.hpp"

#include <set>
namespace gage::scene::systems
{
    Animation::Animation()
    {
    }
    Animation::~Animation()
    {
    }

    void Animation::init()
    {
        for (std::unique_ptr<components::Animator> &animator : animators)
        {
            std::vector<void *> mesh_renderers{};
            animator->node.get_requested_component_accumulate_recursive(typeid(components::MeshRenderer).name(), mesh_renderers);

            for (void *ptrs : mesh_renderers)
            {
                animator->p_mesh_renderers.push_back((components::MeshRenderer *)ptrs);
            }

            log().trace("Animator found {} meshes with skin.", mesh_renderers.size());
        }
    }
    void Animation::update(float delta)
    {
        for (std::unique_ptr<components::Animator> &animator : animators)
        {
            for (const auto mesh_renderer : animator->p_mesh_renderers)
            {
                mesh_renderer->get_animation_buffer().enabled = (animator->current_animation != nullptr);
            }
            if (animator->current_animation == nullptr)
                continue;

            animator->current_time += delta;

            auto get_key_frame_index = [](double current_time, const std::vector<float> &key_frames) -> int32_t
            {
                for (int32_t index = 0; index < ((int64_t)key_frames.size() - 1); index++)
                {
                    if (current_time < key_frames.at(index + 1))
                        return index;
                }
                return 0;
            };

            auto get_scale_factor = [](float current_time_point, float next_time_point, float current_time)
            {
                float scale_factor = 0.0f;
                float current_time_rev_to_current_time_point = current_time - current_time_point;
                float diff = next_time_point - current_time_point;
                scale_factor = current_time_rev_to_current_time_point / diff;

                // scale_factor = std::floor(scale_factor);
                return scale_factor;
            };

            for (const auto &channel : animator->current_animation->pos_channels)
            {
                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(animator->current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), animator->current_time);
                    glm::vec3 position = glm::mix(channel.positions.at(index), channel.positions.at(index + 1), scale_factor);
                    animator->bone_id_to_joint_map.at(channel.target_node)->set_position(position);
                }
            }

            for (const auto &channel : animator->current_animation->scale_channels)
            {

                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(animator->current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), animator->current_time);
                    glm::vec3 scale = glm::mix(channel.scales.at(index), channel.scales.at(index + 1), scale_factor);
                    animator->bone_id_to_joint_map.at(channel.target_node)->set_scale(scale);
                }
            }

            for (const auto &channel : animator->current_animation->rotation_channels)
            {

                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(animator->current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), animator->current_time);
                    glm::quat rotation = glm::slerp(channel.rotations.at(index), channel.rotations.at(index + 1), scale_factor);
                    animator->bone_id_to_joint_map.at(channel.target_node)->set_rotation(rotation);
                }
            }

            animator->current_time = std::fmod(animator->current_time, animator->current_animation->duration);
        }
    }
    void Animation::late_update(float delta)
    {
        for (std::unique_ptr<components::Animator> &animator : animators)
        {
            if (animator->current_animation == nullptr)
                continue;
            for (const auto &[skeleton_id, joint] : animator->skeleton_id_to_joint_map)
            {
                for (const auto mesh_renderer : animator->p_mesh_renderers)
                {
                    mesh_renderer->get_animation_buffer().bone_matrices[skeleton_id] = joint->get_global_transform() * joint->get_inverse_bind_transform();
                }
            }
        }
    }
    void Animation::shutdown()
    {
    }
    void Animation::add_animator(std::unique_ptr<components::Animator> animator)
    {
        animators.push_back(std::move(animator));
    }

    void Animation::set_animator_animation(components::Animator *animator, const std::string &animation)
    {

        std::function<void(std::map<uint32_t, Node *> & out_joints, const data::ModelAnimation &animation, Node *node)> link_bone_id_recursive;
        link_bone_id_recursive = [&link_bone_id_recursive](std::map<uint32_t, Node *> &out_joints, const data::ModelAnimation &animation, Node *node)
        {
            std::set<uint32_t> bone_ids;
            for (const auto &channel : animation.pos_channels)
            {
                bone_ids.insert(channel.target_node);
            }
            for (const auto &channel : animation.rotation_channels)
            {
                bone_ids.insert(channel.target_node);
            }

            for (const auto &channel : animation.scale_channels)
            {
                bone_ids.insert(channel.target_node);
            }

            for (uint32_t bone_id : bone_ids)
            {
                if (bone_id == node->get_bone_id())
                {
                    out_joints.insert({bone_id, node});
                }
            }
            for (const auto &child : node->get_children())
            {
                link_bone_id_recursive(out_joints, animation, child);
            }
        };

        std::function<void(std::map<uint32_t, Node *> & out_joints, const std::map<uint32_t, Node *> &in_joints, const std::vector<uint32_t> &skeleton_joint_indices)> link_skeleton_id;
        link_skeleton_id = [&link_skeleton_id](std::map<uint32_t, Node *> &out_joints, const std::map<uint32_t, Node *> &in_joints, const std::vector<uint32_t> &skeleton_joint_indices)
        {
            uint32_t i = 0;
            for (uint32_t index : skeleton_joint_indices)
            {
                Node *joint = in_joints.at(index);
                out_joints.insert({i, joint});
                i++;
            }
        };

        if (animator->current_animation && animator->current_animation->name.compare(animation) == 0)
        {
            return;
        }
        animator->current_animation = nullptr;
        animator->bone_id_to_joint_map.clear();
        animator->skeleton_id_to_joint_map.clear();
        for (const auto &model_animation : animator->model.animations)
        {
            log().trace("Searching animation: {}", model_animation.name);
            if (model_animation.name.compare(animation) == 0)
            {
                animator->current_animation = &model_animation;
                // Link all joints
                link_bone_id_recursive(animator->bone_id_to_joint_map, model_animation, &animator->node);

                for (const auto mesh_renderer : animator->p_mesh_renderers)
                {
                    link_skeleton_id(animator->skeleton_id_to_joint_map, animator->bone_id_to_joint_map, mesh_renderer->get_skin()->joints);
                }

                log().trace("skeleton_id_to_joint_map.size(): {}", animator->skeleton_id_to_joint_map.size());

                break;
            }
        }
    }
}