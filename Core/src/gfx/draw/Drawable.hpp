#pragma once

#include <vector>
#include <memory>

namespace gage::gfx::bind
{
    class IBindable;
    class IndexBuffer;
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
        void destroy(Graphics& gfx);

        void add_bind(std::unique_ptr<bind::IBindable> bind);
        void add_index_buffer(std::unique_ptr<bind::IndexBuffer> index_buffer);
    private:
        std::vector<std::unique_ptr<bind::IBindable>> bindables{};
        const bind::IndexBuffer* index_buffer{};
    };
};