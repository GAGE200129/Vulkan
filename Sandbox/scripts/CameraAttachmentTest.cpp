#include "CameraAttachmentTest.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/scene/Node.hpp>



namespace gage::scene::components
{
    CameraAttachmentTest::CameraAttachmentTest(SceneGraph& scene, Node& node, gfx::data::Camera& camera) :
        IComponent(scene, node),
        camera(camera)
    {

    }

    void CameraAttachmentTest::init()
    {
        

    }
    void CameraAttachmentTest::update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse)
    {
        auto global_transform = node.get_global_transform();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(global_transform, scale, rotation, translation, skew, perspective);

        camera.position = translation;

    }
}