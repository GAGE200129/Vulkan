#include "pch.hpp"
#include "Map.hpp"

#include <stb/stb_image.h>

#include "Physics/BulletEngine.hpp"

static void processFace(Face &face, int direction, const glm::vec3 &min, const glm::vec3 &max);
static void processTop(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);
static void processBottom(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);
static void processFront(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);
static void processBack(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);
static void processLeft(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);
static void processRight(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v);

void Map::addBox(Box box)
{
    if (gData.boxes.size() >= EngineConstants::MAP_MAX_BOXES)
    {
        spdlog::warn("Maximum number of boxes reached: {}", EngineConstants::MAP_MAX_BOXES);
        return;
    }
    const glm::vec3 min = box.center - box.halfSize;
    const glm::vec3 max = box.center + box.halfSize;

    btBoxShape *shape = new btBoxShape(Utils::glmToBtVec3(box.halfSize));
    btTransform t;
    t.setOrigin(Utils::glmToBtVec3(box.center));
    t.setRotation(btQuaternion(0, 0, 0, 1));
    box.physicsCollider = BulletEngine::createCollisionObject(shape, t);
    

    for (int i = 0; i < 6; i++)
    {
        Face &f = box.faces[i];
        processFace(f, i, min, max);
    }

    gData.boxes.push_back(box);
    gData.boxes.back().physicsCollider->setUserPointer(&gData.boxes.back());
}

static void processFace(Face &face, int direction, const glm::vec3 &min, const glm::vec3 &max)
{
    int bpp;
    if (std::strlen(face.texturePath) == 0)
        return;

    stbi_uc *image = stbi_load(face.texturePath, &face.textureWidth, &face.textureHeight, &bpp, STBI_rgb);
    if (!image)
        return;
    face.avalidable = true;

    MapVertex v[6];
    switch (direction)
    {
    case 0: // Top
    {
        processTop(min, max, face, v);
        break;
    }

    case 1: // Bottom
    {
        processBottom(min, max, face, v);
        break;
    }

    case 2: // Front
    {
        processFront(min, max, face, v);

        break;
    }

    case 3: // Bacl
    {
        processBack(min, max, face, v);
        break;
    }

    case 4: // Left
    {
        processLeft(min, max, face, v);
        break;
    }

    case 5: // Right
    {
        processRight(min, max, face, v);
        break;
    }
    }

    // Load verteex
    VulkanEngine::bufferInitAndTransferToLocalDevice(v, sizeof(MapVertex) * 6,
                                                     vk::BufferUsageFlagBits::eVertexBuffer, face.vertexBuffer);
    face.vertexCount = 6;
    face.texture = Map::getTexture(std::string(face.texturePath));

    stbi_image_free(image);
}

static void processBack(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {1, 0, 0};
    const glm::vec3 biTangent = {0, 1, 0};

    MapVertex &v0 = v[0];
    v0.position = min;
    v0.normal = {0, 0, -1};

    MapVertex &v1 = v[1];
    v1.position = {max.x, min.y, min.z};
    v1.normal = {0, 0, -1};

    MapVertex &v2 = v[2];
    v2.position = {min.x, max.y, min.z};
    v2.normal = {0, 0, -1};

    MapVertex &v3 = v[3];
    v3.position = {min.x, max.y, min.z};
    v3.normal = {0, 0, -1};

    MapVertex &v4 = v[4];
    v4.position = {max.x, min.y, min.z};
    v4.normal = {0, 0, -1};

    MapVertex &v5 = v[5];
    v5.position = {max.x, max.y, min.z};
    v5.normal = {0, 0, -1};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processFront(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {1, 0, 0};
    const glm::vec3 biTangent = {0, 1, 0};

    MapVertex &v0 = v[0];
    v0.position = max;
    v0.normal = {0, 0, 1};

    MapVertex &v1 = v[1];
    v1.position = {max.x, min.y, max.z};
    v1.normal = {0, 0, 1};

    MapVertex &v2 = v[2];
    v2.position = {min.x, max.y, max.z};
    v2.normal = {0, 0, 1};

    MapVertex &v3 = v[3];
    v3.position = {min.x, max.y, max.z};
    v3.normal = {0, 0, 1};

    MapVertex &v4 = v[4];
    v4.position = {max.x, min.y, max.z};
    v4.normal = {0, 0, 1};

    MapVertex &v5 = v[5];
    v5.position = {min.x, min.y, max.z};
    v5.normal = {0, 0, 1};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processRight(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {0, 1, 0};
    const glm::vec3 biTangent = {0, 0, 1};

    MapVertex &v0 = v[0];
    v0.position = max;
    v0.normal = {1, 0, 0};

    MapVertex &v1 = v[1];
    v1.position = {max.x, max.y, min.z};
    v1.normal = {1, 0, 0};

    MapVertex &v2 = v[2];
    v2.position = {max.x, min.y, max.z};
    v2.normal = {1, 0, 0};

    MapVertex &v3 = v[3];
    v3.position = {max.x, max.y, min.z};
    v3.normal = {1, 0, 0};

    MapVertex &v4 = v[4];
    v4.position = {max.x, min.y, min.z};
    v4.normal = {1, 0, 0};

    MapVertex &v5 = v[5];
    v5.position = {max.x, min.y, max.z};
    v5.normal = {1, 0, 0};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processLeft(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {0, 1, 0};
    const glm::vec3 biTangent = {0, 0, 1};

    MapVertex &v0 = v[0];
    v0.position = min;
    v0.normal = {-1, 0, 0};

    MapVertex &v1 = v[1];
    v1.position = {min.x, max.y, min.z};
    v1.normal = {-1, 0, 0};

    MapVertex &v2 = v[2];
    v2.position = {min.x, min.y, max.z};
    v2.normal = {-1, 0, 0};

    MapVertex &v3 = v[3];
    v3.position = {min.x, max.y, min.z};
    v3.normal = {-1, 0, 0};

    MapVertex &v4 = v[4];
    v4.position = {min.x, max.y, max.z};
    v4.normal = {-1, 0, 0};

    MapVertex &v5 = v[5];
    v5.position = {min.x, min.y, max.z};
    v5.normal = {-1, 0, 0};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processBottom(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {1, 0, 0};
    const glm::vec3 biTangent = {0, 0, 1};

    MapVertex &v0 = v[0];
    v0.position = min;
    v0.normal = {0, -1, 0};

    MapVertex &v1 = v[1];
    v1.position = {min.x, min.y, max.z};
    v1.normal = {0, -1, 0};

    MapVertex &v2 = v[2];
    v2.position = {max.x, min.y, min.z};
    v2.normal = {0, -1, 0};

    MapVertex &v3 = v[3];
    v3.position = {min.x, min.y, max.z};
    v3.normal = {0, -1, 0};

    MapVertex &v4 = v[4];
    v4.position = {max.x, min.y, max.z};
    v4.normal = {0, -1, 0};

    MapVertex &v5 = v[5];
    v5.position = {max.x, min.y, min.z};
    v5.normal = {0, -1, 0};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processTop(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    const glm::vec3 tangent = {1, 0, 0};
    const glm::vec3 biTangent = {0, 0, 1};

    MapVertex &v0 = v[0];
    v0.position = max;
    v0.normal = {0, 1, 0};

    MapVertex &v1 = v[1];
    v1.position = {min.x, max.y, max.z};
    v1.normal = {0, 1, 0};

    MapVertex &v2 = v[2];
    v2.position = {max.x, max.y, min.z};
    v2.normal = {0, 1, 0};

    MapVertex &v3 = v[3];
    v3.position = {min.x, max.y, max.z};
    v3.normal = {0, 1, 0};

    MapVertex &v4 = v[4];
    v4.position = {min.x, max.y, min.z};
    v4.normal = {0, 1, 0};

    MapVertex &v5 = v[5];
    v5.position = {max.x, max.y, min.z};
    v5.normal = {0, 1, 0};

    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}
