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

  void update(float delta)
  {
    for (auto &c : mComponents)
      c->update(delta);
  }

  void render()
  {
    for(auto& c : mComponents)
      c->render();
  }

  void debugRender()
  {
    for(auto& c : mComponents)
      c->debugRender();
  }

  void shutdown() noexcept
  {
    for (auto &c : mComponents)
      c->shutdown();
  }

  template <typename T>
  T *getComponent() noexcept
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
  T* addComponent(Args &&...args)
  {
    auto c = std::make_unique<T>(args...);
    c->setGameObject(this);
    T* t = c.get();
    mComponents.emplace_back(std::move(c));
    return t;
  }

public:
  std::string mName;
  std::vector<std::unique_ptr<Component>> mComponents;

  // Static fields
public:
  static GameObject &addGameObject(const std::string &name);
  static void registerLuaScript(lua_State* L);

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

  static void globalUpdate(float delta)
  {
    for (const auto &go : sGameObjects)
    {
      go->update(delta);
    }
  }

  static void globalRender()
  {
    for (const auto &go : sGameObjects)
    {
      go->render();
    }
  }

  //Only call this for the debugger please do not call it in the main game
  static void globalDebugRender()
  {
    for(const auto& go: sGameObjects)
    {
      go->debugRender();
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
