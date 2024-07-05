#include "Model.hpp"

#include "../Graphics.hpp"
#include "../Exception.hpp"

#include "Mesh.hpp"

#include <tiny_gltf.h>
#include <stb_image.h>

namespace tinygltf
{
    bool LoadImageData(tinygltf::Image *image, const int /*image_idx*/,
                       std::string * /*err*/, std::string * /*warn*/,
                       int /*req_width*/, int /*req_height*/,
                       const unsigned char *bytes, int size, void * /*user_data*/)
    {

        int w = 0, h = 0, comp = 0, req_comp = STBI_rgb_alpha;
        // Try to decode image header
        if (stbi_info_from_memory(bytes, size, &w, &h, &comp))
        {
            //*warn += "Unknown image format. STB cannot decode image header for image[" + std::to_string(image_idx) + "] name = \"" + image->name + "\".\n";
            stbi_uc *data = stbi_load_from_memory(bytes, size, &w, &h, &comp, req_comp);

            image->width = w;
            image->height = h;
            image->component = comp;
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            image->as_is = false;
            image->image.resize(w * h * comp);
            std::copy(data, data + w * h * comp, image->image.begin());

            stbi_image_free(data);
        }
        else
        {
            unsigned char image_data[] = 
            {
                0, 255, 0, 255, 255, 0, 0, 255,
                255, 255, 0, 255, 255, 0, 255, 255
            };  
            image->width = 2;
            image->height = 2;
            image->component = 4;
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            image->as_is = false;
            image->image.resize(2 * 2 * 4);
            std::copy(image_data, image_data + 2 * 2 * 4, image->image.begin());
        }

        return true;
    }

    bool WriteImageData(
        const std::string * /* basepath */, const std::string * /* filename */,
        const tinygltf::Image *image, bool /* embedImages */,
        const tinygltf::FsCallbacks * /* fs_cb */, const tinygltf::URICallbacks * /* uri_cb */,
        std::string * /* out_uri */, void * /* user_pointer */)
    {
        assert(false);
    }
}
namespace gage::gfx::draw
{
    Model::Model(Graphics &gfx, const std::string &file_path, Mode mode) : 
        gfx(gfx),
        mode(mode)
    {
        ready = false;
        thread.emplace(&Model::load_async, this, file_path);
    }

    void Model::draw(VkCommandBuffer cmd) const
    {
        if (!ready)
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
        loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
        loader.SetImageWriter(tinygltf::WriteImageData, nullptr);

        bool ret = false;
        switch(mode)
        {
        case Mode::Binary:
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, file_path);
            break;
        case Mode::ASCII:
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, file_path);
            break;
        }

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

        // Materials
        materials.reserve(model.materials.size());
        for (const auto &gltf_material : model.materials)
        {
            materials.emplace_back(gfx, gltf_material);
        }

        this->ready = true;
    }
}