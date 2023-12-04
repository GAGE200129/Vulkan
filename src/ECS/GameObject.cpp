#include "GameObject.hpp"

std::vector<std::unique_ptr<GameObject>> GameObject::sGameObjects;


GameObject& GameObject::addGameObject(const std::string& name)
{
  auto go = std::make_unique<GameObject>(name);
  auto ptr = go.get();
  sGameObjects.emplace_back(std::move(go));
  return *ptr;
}