#pragma once

#include "Vulkan/VulkanEngine.hpp"
#include "Character.hpp"

class Player
{
public:
    Player();
    void update();
    void render() const;
    inline const Camera& getCamera() const { return mCamera ; }
private:
    bool        mFocus;
    Camera      mCamera;
    Character   mCharacter;
};