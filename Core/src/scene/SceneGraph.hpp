#pragma once

#include "Node.hpp"

#include "data/Model.hpp"
#include "systems/Renderer.hpp"
#include "systems/TerrainRenderer.hpp"
#include "systems/Animation.hpp"
#include "systems/Physics.hpp"
#include "systems/Generic.hpp"
#include "systems/MapRenderer.hpp"

#include <vector>
#include <cstdint>
#include <memory>
#include <string_view>



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
    class SceneGraph
    {
    public:
        static constexpr std::string_view ROOT_NAME = "ROOT";
    public:
        SceneGraph(const gfx::Graphics& gfx, phys::Physics& phys, gfx::data::Camera& camera);
        ~SceneGraph();

        void render_imgui();

        void save(const std::string& file_path);
        void init();
        void build_node_transform();

        Node* create_node();
        void* add_component(Node* node, std::unique_ptr<components::IComponent> component);
        


        const data::Model& import_model(const std::string& file_path, data::ModelImportMode mode);
        Node* instanciate_model(const data::Model& model, glm::vec3 initial_position);


        static void make_parent(Node* parent, Node* child);

        const std::vector<std::unique_ptr<Node>>& get_nodes() const;
    private:
        const gfx::Graphics& gfx;
        uint64_t id{0};
    public:
        systems::Renderer renderer;
        systems::TerrainRenderer terrain_renderer;
        systems::MapRenderer map_renderer;
        systems::Animation animation;
        systems::Physics physics;
        systems::Generic generic;
        std::vector<std::unique_ptr<Node>> nodes{};
        std::vector<std::unique_ptr<data::Model>> models{};
    };
}