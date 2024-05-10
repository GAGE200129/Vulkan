#pragma once

#include "Components.hpp"

class ScriptComponent : public Component
{
public:
    ScriptComponent(const std::string &filePath) : mFilePath(filePath) {}
    void init() override;
    void lateInit() override;
    void update(float delta) override;
    void shutdown() noexcept override;

private:
    std::string mFilePath;
    lua_State *L;
};