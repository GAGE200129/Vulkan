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
            glm::vec3 light_position{}; float _padding{};
        } data;
    public:
        GUBO(Graphics& gfx, VmaAllocator allocator);
        void destroy(VmaAllocator allocator);
        ~GUBO() = default;


        void update();
    private:
        VkBuffer buffer{};
        VmaAllocation alloc{};
        VmaAllocationInfo alloc_info{};
    };

}