#pragma once

#include "Components.hpp"

class GameObject
{
public:
    GameObject(const std::string &name) : mName(name) {}
    ~GameObject() = default;

    void init()
    {
        for (auto &c : mComponents)
            c->init();
    }

    void lateInit()
    {
        for (auto &c : mComponents)
            c->lateInit();
    }

    void update()
    {
        for (auto &c : mComponents)
            c->update();
    }

    void render()
    {
        for (auto &c : mComponents)
            c->render();
    }

    void shutdown()
    {
        for (auto &c : mComponents)
            c->shutdown();
    }

    const glm::mat4x4 buildTransform();

    template <typename T>
    T *getComponent()
    {

        for (const auto &c : mComponents)
        {
            T *t = dynamic_cast<T *>(c.get());
            if (t != nullptr)
                return t;
        }
        return nullptr;
    }

    template <typename T>
    T *getRequiredComponent()
    {
        for (const auto &c : mComponents)
        {
            T *t = dynamic_cast<T *>(c.get());
            if (t != nullptr)
                return t;
        }
        throw std::runtime_error("Missing required component !");
        return nullptr;
    }

    template <typename T, typename... Args>
    T *addComponent(Args &&...args)
    {
        auto c = std::make_unique<T>(args...);
        c->setGameObject(this);
        T *t = c.get();
        mComponents.emplace_back(std::move(c));
        return t;
    }

public:
    std::string mName;
    std::vector<std::unique_ptr<Component>> mComponents;

    glm::vec3 mPosition = {0, 0, 0}, mScale = {1, 1, 1};
    glm::quat mRotation = glm::quat(1, 0, 0, 0);

    // Static fields
public:
    static GameObject &addGameObject(const std::string &name);

    static void globalInit()
    {
        for (const auto &go : sGameObjects)
        {
            go->init();
        }

        for (const auto &go : sGameObjects)
        {
            go->lateInit();
        }
    }

    static void globalUpdate()
    {
        for (const auto &go : sGameObjects)
        {
            go->update();
        }
    }

    static void globalRender()
    {
        for (const auto &go : sGameObjects)
        {
            go->render();
        }
    }


    static void globalShutdown()
    {
        for (const auto &go : sGameObjects)
        {
            go->shutdown();
        }
    }

    template <typename T>
    static T *findOfType()
    {
        for (const auto &go : sGameObjects)
        {
            T *c = go->getComponent<T>();
            if (c != nullptr)
                return c;
        }
        return nullptr;
    }

    inline static std::vector<std::unique_ptr<GameObject>> &getGameObjects() { return sGameObjects; }

private:
    static std::vector<std::unique_ptr<GameObject>> sGameObjects;
};
