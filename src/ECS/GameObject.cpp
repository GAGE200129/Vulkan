#include "pch.hpp"
#include "GameObject.hpp"

#include "CharacterControllerComponent.hpp"
#include "RigidBodyComponent.hpp"

#include <imgui.h>

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
    auto luaTransformSetPosition = [](lua_State *L) -> int
    {
        GameObject *go = (GameObject *)lua_touserdata(L, 1);
        go->mPosition = {lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)};
        return 0;
    };

    auto luaTransformGetPosition = [](lua_State *L) -> int
    {
        GameObject *go = (GameObject *)lua_touserdata(L, 1);
        lua_pushnumber(L, go->mPosition.x);
        lua_pushnumber(L, go->mPosition.y);
        lua_pushnumber(L, go->mPosition.z);
        return 3;
    };

    auto luaGameObjectGetComponent = [](lua_State *L) -> int
    {
        static std::map<std::string, std::function<Component *(GameObject * go)>> sComponentMap =
        {
            {
            "character_controller",
            [](GameObject *go)
            { 
                return go->getRequiredComponent<CharacterControllerComponent>(); 
            }
            }
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
    lua_register(L, "transform_set_position", luaTransformSetPosition);
    lua_register(L, "transform_get_position", luaTransformGetPosition);
}

void GameObject::renderImgui() noexcept
{
    ImGui::Text("Transform component");
    bool positionUpdated = false, rotationUpdated = false;
    positionUpdated |= ImGui::DragFloat3("Position", &mPosition.x, 0.1f);
    rotationUpdated |= ImGui::DragFloat4("Rotation", &mRotation.x, 0.01f, -1.0f, 1.0f);

    auto updateRigidBodyTransform = [this](btRigidBody *pBody)
    {
        btTransform &t = pBody->getWorldTransform();
        btVector3 origin = t.getOrigin();
        btQuaternion q = t.getRotation();
        origin.setValue(mPosition.x, mPosition.y, mPosition.z);
        q.setValue(mRotation.x, mRotation.y, mRotation.z, mRotation.w);
        t.setOrigin(origin);
        t.setRotation(q);
        pBody->activate();
    };

    if (rotationUpdated)
    {
        mRotation = glm::normalize(mRotation);
    }

    if (rotationUpdated || positionUpdated)
    {
        RigidBodyComponent *pRigidBody = getComponent<RigidBodyComponent>();
        CharacterControllerComponent *pCharacterController = getComponent<CharacterControllerComponent>();
        if (pRigidBody)
        {
            updateRigidBodyTransform(pRigidBody->mBody);
        }
        if (pCharacterController)
        {
            updateRigidBodyTransform(pCharacterController->mBody);
        }
    }
}

const glm::mat4x4 GameObject::buildTransform() noexcept
{
    glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), mScale);
    glm::mat4 rotateM = glm::mat4(mRotation);
    glm::mat4 positionM = glm::translate(glm::mat4(1.0f), mPosition);

    return positionM * rotateM * scaleM;
}