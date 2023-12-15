#include "pch.hpp"
#include "GameObject.hpp"

#include "TransformComponent.hpp"

std::vector<std::unique_ptr<GameObject>> GameObject::sGameObjects;

GameObject &GameObject::addGameObject(const std::string &name)
{
  auto go = std::make_unique<GameObject>(name);
  auto ptr = go.get();
  sGameObjects.emplace_back(std::move(go));
  return *ptr;
}

void GameObject::registerLuaScript(lua_State *L)
{
  auto luaGameObjectGetComponent = [](lua_State * L) -> int
  {
    static std::map<std::string, std::function<Component *(GameObject * go)>> sComponentMap =
    {
      {"transform", [](GameObject *go)
        { return go->getRequiredComponent<TransformComponent>(); }}
    };

    GameObject *go = (GameObject *)lua_touserdata(L, 1);
    std::string name = lua_tostring(L, 2);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    lua_pushlightuserdata(L, sComponentMap.at(name)(go));
    return 1;
  };
  lua_register(L, "gameobject_get_component", luaGameObjectGetComponent);
}