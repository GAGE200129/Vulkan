#pragma once

#include "../components/Animator.hpp"
#include <vector>
#include <memory>

namespace gage::scene::systems
{
    class Animation
    {
    public:
        Animation();
        ~Animation();
        
        void init();
        void update(float delta);
        void late_update(float delta);

        void shutdown();

        void add_animator(std::unique_ptr<components::Animator> animator);
        static void set_animator_animation(components::Animator* animator, const std::string&  animation);
    private:
        std::vector<std::unique_ptr<components::Animator>> animators; 
    };
}