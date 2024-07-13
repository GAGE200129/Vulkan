#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <cstring>
#include <functional>

namespace gage::utils
{
    class Cvar
    {
    public:
        std::string name{};

        union Data
        {
            uint8_t string[1024];
            float f32;
            uint32_t u32;
            int32_t i32;
        }data {};

    public:
        Cvar(std::string name, Data data = {});


        static std::vector<Cvar*> cvars;
        static void create_cvar(Cvar& cvar_)
        {
            for(const auto& cvar : cvars)
            {
                if(cvar_.name.compare(cvar->name) == 0)
                {
                    assert(false && "CVar already registered !");
                }
            }

            cvars.push_back(&cvar_);
        }

        template<typename T>
        static void set(std::string name, const T& value)
        {
            for(auto& cvar : cvars)
            {
                if(cvar->name.compare(name) == 0)
                {
                    std::memcpy(&cvar->data, &value, sizeof(T));
                    return;
                }
            }

            assert(false && "Cvar not registered !");
        }

        template<typename T>
        static T get(std::string name)
        {
            for(auto& cvar : cvars)
            {
                if(cvar->name.compare(name) == 0)
                {
                    return *(T*)(&cvar->data);
                }
            }

            assert(false && "Cvar not registered !");
            return {};
        }
    };

    
}