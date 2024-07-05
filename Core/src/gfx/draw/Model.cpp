#include "Model.hpp"

#include "../Graphics.hpp"
#include "../Exception.hpp"

#include "Mesh.hpp"

#include <tiny_gltf.h>


namespace gage::gfx::draw
{
    Model::Model(Graphics &gfx, const std::string &file_path) : 
        gfx(gfx)
    {
        ready = false;
        thread.emplace(&Model::load_async, this, file_path);
    }

    void Model::draw(VkCommandBuffer cmd) const
    {
        if(!ready)  
            return;
        nodes.at(root_node).draw(cmd);
    }

    Model::~Model()
    {
        thread.value().join();
    }

    void Model::load_async(const std::string &file_path)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        // tinygltf::Set

        bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, file_path);
        if (!ret)
        {
            throw GraphicsException{"Failed to load model: " + file_path + "| " + warn + "| " + err};
        }
        // Process meshes
        meshes.reserve(model.meshes.size());
        for (const auto &gltf_mesh : model.meshes)
        {
            meshes.emplace_back(gfx, *this, model, gltf_mesh);
        }
        // Process nodes
        nodes.reserve(model.nodes.size());
        root_node = model.scenes.at(model.defaultScene).nodes.at(0);
        for (const auto &gltf_node : model.nodes)
        {
            glm::vec3 position{0, 0, 0};
            glm::quat rotation{1, 0, 0, 0};
            glm::vec3 scale{1, 1, 1};

            assert(gltf_node.matrix.size() == 0);

            if (gltf_node.translation.size() != 0)
            {
                position.x = gltf_node.translation.at(0);
                position.y = gltf_node.translation.at(1);
                position.z = gltf_node.translation.at(2);
            }

            if (gltf_node.scale.size() != 0)
            {
                scale.x = gltf_node.scale.at(0);
                scale.y = gltf_node.scale.at(1);
                scale.z = gltf_node.scale.at(2);
            }

            if (gltf_node.rotation.size() != 0)
            {
                rotation.x = gltf_node.rotation.at(0);
                rotation.y = gltf_node.rotation.at(1);
                rotation.z = gltf_node.rotation.at(2);
                rotation.w = gltf_node.rotation.at(3);
            }

            std::vector<uint32_t> children{};
            for (const auto &child : gltf_node.children)
            {
                children.push_back((uint32_t)child);
            }

            nodes.emplace_back(gfx, *this, position, rotation, scale, std::move(children), gltf_node.mesh);
        }

        //Materials
        materials.reserve(model.materials.size());
        for(const auto& gltf_material : model.materials)
        {
            materials.emplace_back(gfx);
        }

        this->ready = true;
        
    }
}