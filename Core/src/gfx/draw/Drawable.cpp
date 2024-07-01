#include "Drawable.hpp"

#include "../Graphics.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/UniformBuffer.hpp"
#include "../data/DefaultPipeline.hpp"

#include <cassert>

namespace gage::gfx::draw
{
    void Drawable::destroy(Graphics& gfx)
    {
        for(const auto& bind : bindables)
        {
            bind->destroy(gfx);
        }
        if(!get_static_bind().empty())
        {
            for(const auto& bind : get_static_bind())
            {
                bind->destroy(gfx);
            }
            get_static_bind().clear();
        }   
    }
    void Drawable::draw(Graphics& gfx)
    {
        for(const auto& bind : bindables)
        {
            bind->bind(gfx);
        }

        for(const auto& bind : get_static_bind())
        {
            bind->bind(gfx);
        }

        gfx.draw_indexed(index_buffer->get_vertex_count());
    }
    void Drawable::add_bind(std::unique_ptr<bind::IBindable> bind)
    {
        assert(typeid(*bind) != typeid(bind::IndexBuffer));
        bindables.push_back(std::move(bind));

    }
    void Drawable::add_index_buffer(std::unique_ptr<bind::IndexBuffer> index_buffer)
    {
        assert(this->index_buffer == nullptr);
        this->index_buffer = index_buffer.get();
        bindables.push_back(std::move(index_buffer));
    }

}