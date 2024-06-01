#include "pch.hpp"
#include "Debug.hpp"

#include <imgui.h>

#include "Map/Map.hpp"

#include "Input.hpp"
#include "Physics/BulletEngine.hpp"

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
        Map::boxAdd(b);
    }
    if (gData.selectedBox)
    {
        Box *box = gData.selectedBox;
        ImGui::Separator();
        ImGui::Text("Box");

        bool dirty = false;
        dirty |= ImGui::DragFloat3("Center", &box->center.x, 0.1f);
        dirty |= ImGui::DragFloat3("Half size", &box->halfSize.x, 0.1f);

        ImGui::Separator();
        for (int i = 0; i < 6; i++)
        {
            Face &f = box->faces[i];
            ImGui::PushID(i);
            dirty |= ImGui::DragFloat2("Scale", &f.scaleX, 0.01f);
            dirty |= ImGui::InputText("Path", f.texturePath, EngineConstants::PATH_LENGTH);
            ImGui::PopID();
        }
        ImGui::Separator();

        if (dirty)
        {
            Map::boxUpdate(box);
        }
    }

    ImGui::End();
}

void Debug::mapEditorPickBrush()
{
    if (!gData.lockCamera)
        return;
    if (Input::isButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        btVector3 begin(gData.camera.position.x, gData.camera.position.y, gData.camera.position.z);
        btVector3 end = begin + btVector3(gData.cameraForward.x, gData.cameraForward.y, gData.cameraForward.z) * 100.0f;

        btCollisionWorld::ClosestRayResultCallback rayResult(begin, end);

        // Perform raycast
        BulletEngine::getWorld()->rayTest(begin, end, rayResult);
        if (rayResult.hasHit())
        {
            gData.selectedBox = (Box*)rayResult.m_collisionObject->getUserPointer();
        }
        else
        {
            gData.selectedBox = nullptr;
        }
    }
}