#pragma once

#include "../bind/IBindable.hpp"
#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/DescriptorSet.hpp"
#include "Drawable.hpp"

#include <cassert>



namespace gage::gfx::draw
{
    template<typename T>
    class DrawableBase : public Drawable
    {
    protected:
        bool is_static_initialized() const
        {
            return !static_bindables.empty();
        }

        void add_static_bind(std::unique_ptr<bind::IBindable> bind)
        {
            assert(typeid(*bind) != typeid(bind::IndexBuffer));
            static_bindables.push_back(std::move(bind));
        }
        void add_static_index_buffer(std::unique_ptr<bind::IndexBuffer> index_buffer)
        {
            assert(this->index_buffer == nullptr);
            this->index_buffer = index_buffer.get();
            static_bindables.push_back(std::move(index_buffer));
        }
        std::vector<std::unique_ptr<bind::IBindable>>& get_static_bind() override
        {
            return static_bindables;
        }

        template<typename A>
        A* search_static()
        {
            for(const auto& bind : static_bindables)
            {
                if(auto p = dynamic_cast<A*>(bind.get()))
                {
                    return p;
                }
            }

            return nullptr;
        }
        
    private:
        static std::vector<std::unique_ptr<bind::IBindable>> static_bindables;
    };

    template<typename T>
    std::vector<std::unique_ptr<bind::IBindable>> DrawableBase<T>::static_bindables = {};
}