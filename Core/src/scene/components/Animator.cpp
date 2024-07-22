#include <pch.hpp>
#include "Animator.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <set>

#include "MeshRenderer.hpp"

namespace gage::scene::components
{
    Animator::Animator(SceneGraph &scene, Node &node, const data::Model &model, const std::vector<std::unique_ptr<data::ModelAnimation>> &model_animations) : 
        IComponent(scene, node),
        model(model),
        model_animations(model_animations)

    {
    }

    void Animator::init()
    {
        p_mesh_renderer = (MeshRenderer *)node.get_requested_component_recursive(typeid(MeshRenderer).name());
        assert(p_mesh_renderer->get_skin() != nullptr);

    }
    void Animator::update(float delta)
    {
        if (current_animation != nullptr)
        {
            current_time += delta;

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

                //scale_factor = std::foor
                return scale_factor;
            };

            for (const auto &channel : current_animation->pos_channels)
            {
                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), current_time);
                    glm::vec3 position = glm::mix(channel.positions.at(index), channel.positions.at(index + 1), scale_factor);
                    bone_id_to_joint_map.at(channel.target_node)->set_position(position);
                }      
            }

            for (const auto &channel : current_animation->scale_channels)
            {

                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), current_time);
                    glm::vec3 scale = glm::mix(channel.scales.at(index), channel.scales.at(index + 1), scale_factor);
                    bone_id_to_joint_map.at(channel.target_node)->set_scale(scale);
                }

            }

            for (const auto &channel : current_animation->rotation_channels)
            {

                // Interpolate position
                if (channel.time_points.size() >= 2)
                {
                    int index = get_key_frame_index(current_time, channel.time_points);
                    float scale_factor = get_scale_factor(channel.time_points.at(index), channel.time_points.at(index + 1), current_time);
                    glm::quat rotation = glm::slerp(channel.rotations.at(index), channel.rotations.at(index + 1), scale_factor);
                    bone_id_to_joint_map.at(channel.target_node)->set_rotation(rotation);
                }

            }

            for(const auto& [skeleton_id, joint] : skeleton_id_to_joint_map)
            {       
                p_mesh_renderer->get_bone_matrices()[skeleton_id] = joint->get_global_transform() * joint->get_inverse_bind_transform();
            }

            current_time = std::fmod(current_time, current_animation->duration);
        }
    }

    void Animator::render_depth(VkCommandBuffer, VkPipelineLayout)
    {
    }
    void Animator::render_geometry(VkCommandBuffer, VkPipelineLayout)
    {
    }

    void Animator::shutdown()
    {
    }

    void Animator::set_current_animation(const std::string &name)
    {
        bone_id_to_joint_map.clear();
        current_animation = nullptr;

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

            for(uint32_t bone_id : bone_ids)
            {
                if(bone_id == node->get_bone_id())
                {
                    out_joints.insert({bone_id, node});
                }
            }
            for (const auto &child : node->get_children())
            {
                link_bone_id_recursive(out_joints, animation, child);
            }
        };

        std::function<void(std::map<uint32_t, Node *> & out_joints, const std::map<uint32_t, Node *> & in_joints, const std::vector<uint32_t>& skeleton_joint_indices)> link_skeleton_id;
        link_skeleton_id = [&link_skeleton_id](std::map<uint32_t, Node *> & out_joints, const std::map<uint32_t, Node *> & in_joints, const std::vector<uint32_t>& skeleton_joint_indices)
        {
            uint32_t i = 0;
            for(uint32_t index : skeleton_joint_indices)
            {
                Node* joint = in_joints.at(index);
                out_joints.insert({i, joint});
                i++;
            }
        };

        for (const auto &model_animation : model_animations)
        {
            log().trace("Searching animation: {}", model_animation->name);
            if (model_animation->name.compare(name) == 0)
            {
                current_animation = model_animation.get();
                // Link all joints
                link_bone_id_recursive(bone_id_to_joint_map, *model_animation, &this->node);
                link_skeleton_id(skeleton_id_to_joint_map, bone_id_to_joint_map, p_mesh_renderer->get_skin()->joints);
                break;
            }
        }
    }

}