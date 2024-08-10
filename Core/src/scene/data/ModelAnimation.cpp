#include <pch.hpp>
#include "ModelAnimation.hpp"

#include "../scene.hpp"


namespace gage::scene::data
{
    ModelAnimation::ModelAnimation(const tinygltf::Model &gltf_model, const tinygltf::Animation &gltf_animation)
    {
        name = gltf_animation.name;
        log().trace("Animation name: {}", gltf_animation.name);

        auto extract_buffer_from_accessor = [&](const tinygltf::Accessor &accessor)
            -> std::vector<unsigned char>
        {
            const auto &buffer_view = gltf_model.bufferViews.at(accessor.bufferView);
            const auto &buffer = gltf_model.buffers.at(buffer_view.buffer);

            std::vector<unsigned char> result(buffer_view.byteLength);
            std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

            return result;
        };

        duration = std::numeric_limits<float>::min();
        for (const auto &gltf_channel : gltf_animation.channels)
        {
            const auto &gltf_sampler = gltf_animation.samplers.at(gltf_channel.sampler);
            const auto &time_point_accessor = gltf_model.accessors.at(gltf_sampler.input);
            auto time_points = extract_buffer_from_accessor(time_point_accessor); // .size() in bytes
            duration = std::max(duration, time_point_accessor.maxValues.at(0));
            if (gltf_channel.target_path.compare("translation") == 0)
            {
                data::ModelAnimation::PositionChannel channel{};
                channel.target_node = gltf_channel.target_node;
                const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                channel.positions.resize(data_accessor.count);         // .size() in n * glm::vec3 ( 12 bytes )
                // log().info("Time point size: {} bytes, data size: {} bytes", time_points.size(),  data.size());

                std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));
                std::memcpy(channel.positions.data(), data.data(), data_accessor.count * sizeof(glm::vec3));

                pos_channels.push_back(std::move(channel));
            }
            else if (gltf_channel.target_path.compare("scale") == 0)
            {
                data::ModelAnimation::ScaleChannel channel{};
                channel.target_node = gltf_channel.target_node;
                const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                channel.scales.resize(data_accessor.count);            // .size() in n * glm::vec3 ( 12 bytes )
                // log().info("Time point size: {} bytes, data size: {} bytes", time_points.size(),  data.size());

                std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));
                std::memcpy(channel.scales.data(), data.data(), data_accessor.count * sizeof(glm::vec3));

                scale_channels.push_back(std::move(channel));
            }

            else if (gltf_channel.target_path.compare("rotation") == 0)
            {
                data::ModelAnimation::RotationChannel channel{};
                channel.target_node = gltf_channel.target_node;
                const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));

                if (data_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    channel.rotations.reserve(data_accessor.count);
                    for (uint32_t i = 0; i < data_accessor.count; i++)
                    {
                        float x = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 0);
                        float y = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 1);
                        float z = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 2);
                        float w = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 3);
                        channel.rotations.push_back(glm::quat{w, x, y, z});
                    }
                }
                else
                {
                    log().critical("Animation rotation channel is not in float ! TO DO LIST !");
                    throw SceneException{};
                }

                rotation_channels.push_back(std::move(channel));
            }
        }
        log().trace("Animation duration: {}", duration);
    }
    ModelAnimation::~ModelAnimation()
    {
    }
}