#pragma once

#include <vector>
#include <memory>

#include <glm/mat4x4.hpp>

namespace gage::gfx::bind
{
    class IBindable;
    class IndexBuffer;
    class UniformBuffer;
}

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::draw
{
    class Drawable
    {
    public:
        Drawable() = default;
        virtual ~Drawable() = default;
        Drawable(const Drawable&) = delete;

        void draw(Graphics& gfx);

        void add_bind(std::unique_ptr<bind::IBindable> bind);
        void add_index_buffer(std::unique_ptr<bind::IndexBuffer> index_buffer);

        virtual glm::mat4 get_world_transform() const = 0;
        virtual void update(float dt) = 0;
    protected:
        virtual std::vector<std::unique_ptr<bind::IBindable>>& get_static_bind() = 0;
    protected:
        std::vector<std::unique_ptr<bind::IBindable>> bindables{};
        const bind::IndexBuffer* index_buffer{};
    };
};