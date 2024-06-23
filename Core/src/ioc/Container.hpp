#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <functional>
#include <memory>

namespace gage::ioc
{
    template<typename T>
    concept Paramerized = requires() {
        { typename T::IoCParams{} };
    };

    template<typename T>
    concept NotParamerized = !Paramerized<T>;

    class Container
    {
    public:
        template <class T>
        using Generator = std::function<std::shared_ptr<T>()>;
        template <class T>
        using ParameterizedGenerator = std::function<std::shared_ptr<T>(typename T::IoCParams params)>;

        template <NotParamerized T>
        void register_factory(Generator<T> generator)
        {
            this->factory_map[typeid(T)] = generator;
        }

        template <Paramerized T>
        void register_factory(ParameterizedGenerator<T> generator)
        {
            this->factory_map[typeid(T)] = generator;
        }

        template <NotParamerized T>
        std::shared_ptr<T> resolve()
        {
            return resolve_internal<T, Generator<T>>();
        }

        template <Paramerized T>
        std::shared_ptr<T> resolve(typename T::IoCParams&& params = {})
        {
           return resolve_internal<T, ParameterizedGenerator<T>>(std::forward<typename T::IoCParams>(params));
        }
        

        static Container &get()
        {
            static Container instance;
            return instance;
        }

    private:
        template<typename T, typename G, typename...P>
        std::shared_ptr<T> resolve_internal(P&&...arg) 
        {
            if (const auto i = this->factory_map.find(typeid(T)); i != this->factory_map.end())
            {
                try
                {
                    return std::any_cast<G>(i->second)(std::forward<P>(arg)...);
                }
                catch (const std::bad_any_cast&)
                {
                    std::stringstream ss;
                    ss << "Bad cast: " << i->second.type().name() << " to: " << typeid(G).name();
                    throw std::runtime_error{ss.str()};
                }
            }
            else
            {
                throw std::runtime_error{
                    "Could not find generator for type !"};
            }
        }


    private:
        std::unordered_map<std::type_index, std::any> factory_map;
    };
}