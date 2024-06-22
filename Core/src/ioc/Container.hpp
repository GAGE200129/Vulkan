#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <functional>
#include <memory>

namespace gage::ioc
{
    class Container
    {
    public:
        template <class T>
        using Generator = std::function<std::shared_ptr<T>()>;

        template <class T>
        void register_factory(Generator<T> generator)
        {
            this->factory_map[typeid(T)] = generator;
        }

        template <class T>
        std::shared_ptr<T> resolve()
        {
            if (const auto i = this->factory_map.find(typeid(T)); i != this->factory_map.end())
            {
                return std::any_cast<Generator<T>>(i->second)();
            }
            else
            {
                throw std::runtime_error{
                    "Could not find generator for type !"};
            }
        }

        static Container& get() {
            static Container instance;
            return instance;
        }

    private:
        std::unordered_map<std::type_index, std::any> factory_map;
    };
}