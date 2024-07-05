#include "Material.hpp"

#include "../Graphics.hpp"

namespace gage::gfx::draw
{
    Material::Material(Graphics& gfx) :
        gfx(gfx),
        uniform_buffer(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBuffer), &uniform_buffer_data)
    {
        descriptor_set = gfx.get_default_pipeline().allocate_instance_set(sizeof(UniformBuffer), uniform_buffer.get_buffer_handle());

    }


    Material::~Material()
    {
        gfx.get_default_pipeline().free_instance_set(descriptor_set);
    }

    VkDescriptorSet Material::get_desc_set() const
    {
        return descriptor_set;
    }
}