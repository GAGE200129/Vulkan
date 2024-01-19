#include "pch.hpp"
#include "EntityInspector.hpp"

#include <imgui.h>

#include "ECS/GameObject.hpp"

void EntityInspector::renderImGui()
{
  static GameObject *sSelectedGO = nullptr;
  
  auto &gameObjects = GameObject::getGameObjects();
  ImGui::Begin("EntityList");

  if (ImGui::TreeNode("Entities"))
  {
    size_t id = 0;
    for (const auto &go : gameObjects)
    {
      ImGui::PushID(id);
      if (ImGui::Selectable(go->mName.c_str(), go.get() == sSelectedGO))
      {
        sSelectedGO = go.get();
      }
      ImGui::PopID();

      id++;
    }

    ImGui::TreePop();
  }

  ImGui::End();

  ImGui::Begin("Inspector");
  if (sSelectedGO)
  {
    for(const auto& c : sSelectedGO->mComponents)
    {
      c->renderImGui();
      ImGui::Separator();
    } 
  }
  ImGui::End();
}