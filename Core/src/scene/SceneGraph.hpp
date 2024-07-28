#pragma once

#include "Node.hpp"

#include "data/Model.hpp"
#include "systems/Renderer.hpp"
#include "systems/Animation.hpp"
#include "systems/Physics.hpp"
#include "systems/Generic.hpp"

#include <vector>
#include <cstdint>
#include <memory>
#include <string_view>

#include <Core/src/utils/Exception.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::phys
{
    class Physics;
}

namespace tinygltf
{
    class Model;
    class Mesh;
    class Animation;
    class Skin;
    class Material;
}

namespace gage::hid
{
    class Keyboard;
    class Mouse;
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
        enum class SystemType
        {
            Renderer,
            Animation,
            Physics,
            Generic
        };
        static constexpr std::string_view ROOT_NAME = "ROOT";
    public:
        SceneGraph(gfx::Graphics& gfx, phys::Physics& phys);
        ~SceneGraph();

        void init();
        void build_node_transform();

        Node* create_node();
        Node* find_node(uint64_t id);
        void add_component(Node* node, SystemType type, std::unique_ptr<components::IComponent> component);
        


        const data::Model& import_model(const std::string& file_path, ImportMode mode);
        Node* instanciate_model(const data::Model& model, glm::vec3 initial_position);


        static void make_parent(Node* parent, Node* child);

        const std::vector<std::unique_ptr<Node>>& get_nodes() const;
        systems::Renderer& get_renderer();
        systems::Animation& get_animation();
        systems::Physics& get_physics();
        systems::Generic& get_generic();
    private:
        void process_model_mesh(const tinygltf::Model& gltf_model, const tinygltf::Mesh& gltf_mesh, data::ModelMesh& mesh);
        void process_model_material(const tinygltf::Model& gltf_model, const tinygltf::Material& gltf_material, data::ModelMaterial& material);
        void process_model_animation( const tinygltf::Model& gltf_model, const tinygltf::Animation& gltf_animation, data::ModelAnimation& animation);
        void process_model_skin(const tinygltf::Model& gltf_model, const tinygltf::Skin& gltf_skin, data::ModelSkin& skin);
        void process_model_calculate_inverse_bind_transform(data::Model& model, data::ModelNode& root);
    private:
        systems::Renderer renderer;
        systems::Animation animation;
        systems::Physics physics;
        systems::Generic generic;
        gfx::Graphics& gfx;
        uint64_t id{0};
        std::vector<std::unique_ptr<Node>> nodes{};
        std::vector<std::unique_ptr<data::Model>> models{};
    };
}