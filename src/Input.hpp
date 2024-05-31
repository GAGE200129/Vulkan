#pragma once

struct GLFWwindow;
struct lua_State;

struct InputData
{
    GLFWwindow * window;
    double prevMouseX, prevMouseY;
    double dx, dy;
    std::bitset<500> keysPressed, prevKeyPressed;
    std::bitset<6> buttonPressed, prevButtonPressed;
};

namespace Input
{
    void init(GLFWwindow *window);
    void update();
    void registerLuaScript(lua_State *L);
    bool isKeyDown(int key);
    bool isButtonDown(int button);
    bool isKeyDownOnce(int key);
    bool isButtonDownOnce(int button);
    void setCursorMode(bool locked);
    double getDx();
    double getDy();
    void registerScriptKeys(lua_State *L);

    extern InputData gData;
};