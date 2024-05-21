#pragma once

class GameObject;
class Component
{
public:
    Component() = default;
    virtual ~Component() = default;

    virtual void renderImGui() {}
    virtual void init() {}
    virtual void lateInit() {}
    virtual void update() {}
    virtual void render() {}
    virtual void shutdown()  {}

    const GameObject &getGameObject() const  { return *mGameObject; }
    GameObject &getGameObject() { return *mGameObject; }
    void setGameObject(GameObject *go) { mGameObject = go; }

protected:
    GameObject *mGameObject;
};