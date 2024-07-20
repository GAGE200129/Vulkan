#pragma once

#include "Node.hpp"

#include "data/Model.hpp"

#include <vector>
#include <cstdint>
#include <memory>

#include <Core/src/utils/Exception.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace tinygltf
{
    class Model;
    class Mesh;
}

namespace gage::scene
{

    class SceneException : public utils::Exception{ using Exception::Exception; };

    class SceneGraph
    {
    public:
        enum class ImportMode
        {
            Binary,
            ASCII
        };
    public:
        SceneGraph();
        ~SceneGraph();

        void update(float delta);
        void render(gfx::Graphics& gfx, VkCommandBuffer cmd, VkPipelineLayout layout);

        Node* create_node();
        Node* find_node(uint64_t id);
        uint64_t find_node_index(uint64_t id);


        const data::Model* import_model(gfx::Graphics& gfx, const std::string& file_path, ImportMode mode);
        void instanciate_model(const data::Model* model);


        static void make_parent(Node* parent, Node* child);

        const std::vector<std::unique_ptr<Node>>& get_nodes() const;
    private:
        void process_model_mesh(gfx::Graphics& gfx, const tinygltf::Model& gltf_model, const tinygltf::Mesh& gltf_mesh, data::ModelMesh* mesh);
        void process_model_material(gfx::Graphics& gfx, const tinygltf::Model& gltf_model, const tinygltf::Material& gltf_material, data::ModelMaterial* material);
    private:
        uint64_t id{0};
        std::vector<std::unique_ptr<Node>> nodes{};
        std::vector<std::unique_ptr<data::Model>> models{};
    };
}