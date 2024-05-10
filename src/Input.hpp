#pragma once

struct GLFWwindow;
struct lua_State;
class Input
{
public:
    static void init(GLFWwindow *window) noexcept;
    static void update();

    static void registerLuaScript(lua_State *L);
    static bool isKeyDown(int key) noexcept;
    static bool isKeyDownOnce(int key) noexcept;
    static void lockCursor() noexcept;
    static void unlockCursor() noexcept;
    inline static double getDx() noexcept { return mDx; }
    inline static double getDy() noexcept { return mDy; }

    static void registerScriptKeys(lua_State *L);

private:
    static void keyPressedFn(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    static GLFWwindow *mWindow;
    static double mPrevMouseX, mPrevMouseY;
    static double mDx, mDy;
    static std::bitset<500> mKeysPressed, mPrevKeyPressed;
};