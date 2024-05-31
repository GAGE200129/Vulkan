#include "pch.hpp"
#include "Debug.hpp"

#include <imgui.h>

#include "Map/Map.hpp"

void Debug::mapEditorRenderImgui()
{
    ImGui::Begin("Map editor");
    if (ImGui::Button("Spawn brush"))
    {
        Box b;
        b.center = gData.camera.position;
        b.halfSize = {0.5f, 0.5f, 0.5f};
        std::strncpy(b.faces[0].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[0].scaleX = 0.01f;
        b.faces[0].scaleY = 0.01f;

        std::strncpy(b.faces[1].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[1].scaleX = 0.01f;
        b.faces[1].scaleY = 0.01f;

        std::strncpy(b.faces[2].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[2].scaleX = 0.01f;
        b.faces[2].scaleY = 0.01f;

        std::strncpy(b.faces[3].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[3].scaleX = 0.01f;
        b.faces[3].scaleY = 0.01f;

        std::strncpy(b.faces[4].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[4].scaleX = 0.01f;
        b.faces[4].scaleY = 0.01f;

        std::strncpy(b.faces[5].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
        b.faces[5].scaleX = 0.01f;
        b.faces[5].scaleY = 0.01f;
        Map::addBox(b);
    }
    ImGui::End();
}