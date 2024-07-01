#pragma once

#include <cstddef>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace gage::gfx::data
{
    enum class ElementType
    {
        Vec2,
        Vec3,
        Vec4,
        Count,
    };

    template <ElementType>
    struct VertexElementMap;

    template <>
    struct VertexElementMap<ElementType::Vec2>
    {
        using type = glm::vec2;
        static constexpr VkFormat format = VK_FORMAT_R32G32_SFLOAT;
    };

    template <>
    struct VertexElementMap<ElementType::Vec3>
    {
        using type = glm::vec3;
        static constexpr VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
    };

    template <>
    struct VertexElementMap<ElementType::Vec4>
    {
        using type = glm::vec4;
        static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
    };

    class VertexElement
    {
    public:
        static constexpr size_t size_of_type_in_bytes(ElementType type)
        {
            switch (type)
            {
            case ElementType::Vec2:
                return sizeof(VertexElementMap<ElementType::Vec2>::type);
            case ElementType::Vec3:
                return sizeof(VertexElementMap<ElementType::Vec3>::type);
            case ElementType::Vec4:
                return sizeof(VertexElementMap<ElementType::Vec4>::type);
            default:
                assert(false && "Unknown element type !");
            }
        }
        VkVertexInputAttributeDescription get_vulkan_description(uint32_t location) const
        {
            switch (type)
            {
            case ElementType::Vec2:
                return get_vulkan_description<ElementType::Vec2>(location, get_offset());
            case ElementType::Vec3:
                return get_vulkan_description<ElementType::Vec3>(location, get_offset());
            case ElementType::Vec4:
                return get_vulkan_description<ElementType::Vec4>(location, get_offset());
            default:
                assert(false && "Unknown element type !");
            }
        }

        template <ElementType Type>
        VkVertexInputAttributeDescription get_vulkan_description(uint32_t location, size_t offset) const
        {
            VkVertexInputAttributeDescription attribute = {};
            attribute.binding = 0;
            attribute.location = location;
            attribute.format = VertexElementMap<Type>::format;
            attribute.offset = offset;
            return attribute;
        }

        VertexElement(ElementType type, size_t offset) : type(type), offset(offset) {}

        size_t get_size_in_bytes() const
        {
            return size_of_type_in_bytes(type);
        }
        size_t get_offset() const
        {
            return offset;
        }

        size_t get_offset_after() const
        {
            return offset + get_size_in_bytes();
        }

        ElementType get_type() const
        {
            return type;
        }

    private:
        ElementType type{};
        size_t offset{};
    };

    class VertexLayout
    {
    public:
        template <ElementType Type>
        const VertexElement &get() const
        {
            for (auto &e : elements)
            {
                if (e.get_type() == Type)
                {
                    return e;
                }
            }
            assert(false && "Unknown element type !");
            return elements.front();
        }

        const VertexElement &get(size_t i) const
        {
            return elements.at(i);
        }

        VertexLayout &emplace_back(ElementType type)
        {
            elements.emplace_back(type, this->size_in_bytes());
            return *this;
        }

        size_t size_in_bytes() const
        {
            return elements.empty() ? 0 : elements.back().get_offset_after();
        }

        size_t element_count() const
        {
            return elements.size();
        }

        std::vector<VkVertexInputAttributeDescription> get_vulkan_attributes() const
        {
            std::vector<VkVertexInputAttributeDescription> result;
            result.reserve(elements.size());

            uint32_t location = 0;
            for (const auto &e : elements)
            {
                result.push_back(e.get_vulkan_description(location));
                location++;
            }
            return result;
        }

        VkVertexInputBindingDescription get_vulkan_binding() const
        {
            VkVertexInputBindingDescription main_binding{};
            main_binding.binding = 0;
            main_binding.stride = size_in_bytes();
            main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return main_binding;
        }


    private:
        std::vector<VertexElement> elements{};
    };

    class VertexView
    {
    public:
        VertexView(void *data, const VertexLayout &layout) : data(data), layout(layout) {}

        template <ElementType Type>
        const auto &get_attr()
        {
            const auto &element = layout.get<Type>();
            auto p_attribute = (unsigned char *)data + element.get_offset();
            return *reinterpret_cast<VertexElementMap<Type>::type *>(p_attribute);
        }

        template <typename Dest, typename Src>
        void set_attr(void *attr, Src &&val)
        {
            if constexpr (std::is_assignable<Dest, Src>::value)
            {
                *(Dest *)attr = val;
            }
            else
            {
                static_assert("Parameter attribute error !");
            }
        }

        template <typename First, typename... Rest>
        void set_attr(size_t i, First &&first, Rest &&...rest)
        {
            set_attr(i, std::forward<First>(first));
            set_attr(i + 1, std::forward<Rest>(rest)...);
        }

        template <typename T>
        void set_attr(size_t i, T &&val)
        {
            const auto &element = layout.get(i);
            auto p_attribute = (unsigned char *)data + element.get_offset();
            switch (element.get_type())
            {
            case ElementType::Vec2:
                set_attr<VertexElementMap<ElementType::Vec2>::type>(p_attribute, std::forward<T>(val));
                break;
            case ElementType::Vec3:
                set_attr<VertexElementMap<ElementType::Vec3>::type>(p_attribute, std::forward<T>(val));
                break;
            case ElementType::Vec4:
                set_attr<VertexElementMap<ElementType::Vec4>::type>(p_attribute, std::forward<T>(val));
                break;
            default:
                assert(false && "Invalid attribute !");
            }
        }

    private:
        void *data;
        const VertexLayout &layout;
    };

    class RawVertexArray
    {
    public:
        RawVertexArray(VertexLayout layout) : layout(std::move(layout)) {}

        const VertexLayout &get_layout() const
        {
            return layout;
        }

        size_t size_in_bytes() const
        {
            return buffer.size();
        }

        size_t count() const
        {
            return buffer.size() / layout.size_in_bytes();
        }


        const void *data() const
        {
            return buffer.data();
        }

        template <typename... Params>
        void emplace_back(Params &&...params)
        {
            assert(sizeof...(params) == layout.element_count() && "Param mismatch !");
            buffer.resize(buffer.size() + layout.size_in_bytes());
            back().set_attr(0u, std::forward<Params>(params)...);
        }

        VertexView back()
        {
            assert(buffer.size() != 0);
            return VertexView{buffer.data() + buffer.size() - layout.size_in_bytes(), layout};
        }

        VertexView front()
        {
            assert(buffer.size() != 0);
            return VertexView{buffer.data(), layout};
        }

        VertexView at(size_t i)
        {
            assert(i < count());
            return VertexView{buffer.data() + layout.size_in_bytes() * i, layout};
        }

    private:
        std::vector<unsigned char> buffer{};
        VertexLayout layout{};
    };
}