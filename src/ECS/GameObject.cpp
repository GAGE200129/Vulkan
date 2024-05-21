#include "pch.hpp"
#include "GameObject.hpp"



std::vector<std::unique_ptr<GameObject>> GameObject::sGameObjects;

GameObject &GameObject::addGameObject(const std::string &name)
{
    auto go = std::make_unique<GameObject>(name);
    auto ptr = go.get();
    sGameObjects.emplace_back(std::move(go));
    return *ptr;
}


const glm::mat4x4 GameObject::buildTransform()
{
    glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), mScale);
    glm::mat4 rotateM = glm::mat4(mRotation);
    glm::mat4 positionM = glm::translate(glm::mat4(1.0f), mPosition);

    return positionM * rotateM * scaleM;
}