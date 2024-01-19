#include <pch.hpp>
#include "MapEditor.hpp"

#include <imgui.h>

void MapEditor::renderImGui(Map& map)
{
  ImGui::Begin("MapEditor");
  if(ImGui::Button("New brush"))
  {
    map.addBrush({0, 0, 0});
  }
  ImGui::End();
}