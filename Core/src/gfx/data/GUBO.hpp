#pragma once

#include <vk_mem_alloc.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class GUBO
    {
        friend class gage::gfx::Graphics;
    public:
        struct Data
        {
            glm::mat4x4 projection{};
            glm::mat4x4 view{};
            glm::vec3 camera_position; float _padding{};
            glm::vec3 point_light_position{}; float _padding2{};
            glm::vec4 ambient{0.15f, 0.15f, 0.15f, 1.0f};
            glm::vec4 diffuse_color{1.0f,1.0f, 1.0f, 1.0f};
            float diffuse_intensity{1.0f};
            float att_constant{1.0f};
            float att_linear{0.045f};
            float att_exponent{0.0075f};
        } data;
    public:
        GUBO(VmaAllocator allocator);
        void destroy(VmaAllocator allocator);
        ~GUBO() = default;


        void update();

        uint32_t get_size() const;
        VkBuffer get() const;
    private:
        VkBuffer buffer{};
        VmaAllocation alloc{};
        VmaAllocationInfo alloc_info{};
    };

}