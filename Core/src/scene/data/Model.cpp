#include <pch.hpp>
#include "Model.hpp"

#include "../scene.hpp"

namespace gage::scene::data
{
    Model::Model(const gfx::Graphics& gfx, const systems::Renderer& renderer, const std::string &file_path, ModelImportMode mode)
    {
        log().info("Importing scene: {}", file_path);
        this->name = file_path;

        tinygltf::Model gltf_model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
        loader.SetImageWriter(tinygltf::WriteImageData, nullptr);

        bool ret = false;
        switch (mode)
        {
        case ModelImportMode::Binary:
            ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, file_path);
            break;
        case ModelImportMode::ASCII:
            ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, file_path);
            break;
        }

        if (!ret)
        {
            log().critical("Failed to import scene: {} | {} | {}", file_path, warn, err);
            throw SceneException{"Failed to import scene: " + file_path + "| " + warn + "| " + err};
        }
        this->root_node = gltf_model.scenes.at(gltf_model.defaultScene).nodes.at(0);

        // Import nodes
        this->nodes.reserve(gltf_model.nodes.size());
        for (uint32_t i = 0; i < gltf_model.nodes.size(); i++)
        {
            this->nodes.emplace_back(gltf_model.nodes.at(i), i);
        }

        // Process mesh
        this->meshes.reserve(gltf_model.meshes.size());
        for (const auto &gltf_mesh : gltf_model.meshes)
        {
            this->meshes.emplace_back(gfx, gltf_model, gltf_mesh);
        }

        // Process material
        this->materials.reserve(gltf_model.materials.size());
        for (const auto &gltf_material : gltf_model.materials)
        {
            this->materials.emplace_back(gfx, renderer, gltf_model, gltf_material);
        }

        // Process animations
        this->animations.reserve(gltf_model.animations.size());
        for (const auto &gltf_animation : gltf_model.animations)
        {
            this->animations.emplace_back(gltf_model, gltf_animation);
        }

        // Process skins
        this->skins.reserve(gltf_model.skins.size());
        for (const auto &gltf_skin : gltf_model.skins)
        {
            auto joints = std::vector<uint32_t>();
            for (const auto &joint : gltf_skin.joints)
            {
                joints.push_back(joint);
            }
            this->skins.push_back(std::move(joints));
        }

        // Calculate inverse bind transform for skinning
        {
            std::function<void(data::ModelNode & node, glm::mat4x4 accumulated_transform)> traverse_scene_graph_recursive;
            traverse_scene_graph_recursive = [&](data::ModelNode &node, glm::mat4x4 accumulated_transform)
            {
                // Build node model transform
                accumulated_transform = glm::translate(accumulated_transform, node.position);
                accumulated_transform = glm::scale(accumulated_transform, node.scale);
                accumulated_transform *= glm::mat4x4(node.rotation);
                node.inverse_bind_transform = glm::inverse(accumulated_transform); // get inverse

                for (const auto &child : node.children)
                {
                    traverse_scene_graph_recursive(this->nodes.at(child), accumulated_transform);
                }
            };

            traverse_scene_graph_recursive(this->nodes.at(this->root_node), glm::mat4x4(1.0f));
        }
    }
    Model::~Model()
    {

    }
}