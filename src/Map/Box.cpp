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
static void processTextureCoord();

static void boxInitCollider(Box &box);

void Map::boxAdd(Box box)
{
    if (gData.boxes.size() >= EngineConstants::MAP_MAX_BOXES)
    {
        spdlog::warn("Maximum number of boxes reached: {}", EngineConstants::MAP_MAX_BOXES);
        return;
    }
    const glm::vec3 min = box.center - box.halfSize;
    const glm::vec3 max = box.center + box.halfSize;

    boxInitCollider(box);

    for (int i = 0; i < 6; i++)
    {
        Face &f = box.faces[i];
        processFace(f, i, min, max);
    }

    gData.boxes.push_back(box);
    gData.boxes.back().physicsCollider->setUserPointer(&gData.boxes.back());
}

void Map::boxUpdate(Box *box)
{
    const glm::vec3 min = box->center - box->halfSize;
    const glm::vec3 max = box->center + box->halfSize;

    BulletEngine::getWorld()->removeCollisionObject(box->physicsCollider);

    delete box->physicsCollider->getCollisionShape();
    delete box->physicsCollider;

    boxInitCollider(*box);

    for (int i = 0; i < 6; i++)
    {
        Face &f = box->faces[i];
        if (f.avalidable)
        {
            VulkanEngine::bufferCleanup(f.vertexBuffer);
        }
        processFace(f, i, min, max);
    }
}

void Map::boxSave(std::ofstream &stream, const Box &box)
{
    stream << "box "
           << box.center.x << " "
           << box.center.y << " "
           << box.center.z << " "

           << box.halfSize.x << " "
           << box.halfSize.y << " "
           << box.halfSize.z << " ";

    for (int i = 0; i < 6; i++)
    {
        const Face &face = box.faces[i];
        stream << face.texturePath << " "
               << face.scaleX << " "
               << face.scaleY << " ";
    }

    stream << "\n";
}
void Map::boxLoad(const std::vector<std::string> &tokens, Box &box)
{
    
    box.center.x = std::stof(tokens[1]);
    box.center.y = std::stof(tokens[2]);
    box.center.z = std::stof(tokens[3]);

    box.halfSize.x = std::stof(tokens[4]);
    box.halfSize.y = std::stof(tokens[5]);
    box.halfSize.z = std::stof(tokens[6]);

    int j = 0;
    for (int i = 0; i < 6; i++)
    {
        Face& face = box.faces[i];
        std::strncpy(face.texturePath, tokens[j + 7].c_str(), EngineConstants::PATH_LENGTH);
        face.scaleX = std::stof(tokens[j + 8]);
        face.scaleY = std::stof(tokens[j + 9]);
        j += 3;
    }

}

static void boxInitCollider(Box &box)
{
    btBoxShape *s = new btBoxShape(Utils::glmToBtVec3(box.halfSize));
    btTransform t;
    t.setOrigin(Utils::glmToBtVec3(box.center));
    t.setRotation(btQuaternion(0, 0, 0, 1));
    btCollisionObject *object = new btCollisionObject();
    object->setWorldTransform(t);
    object->setCollisionShape(s);
    object->setFriction(1.0);
    object->setUserPointer(&box);

    BulletEngine::getWorld()->addCollisionObject(object);
    box.physicsCollider = object;
}

static void processFace(Face &face, int direction, const glm::vec3 &min, const glm::vec3 &max)
{
    int bpp;
    if (std::strlen(face.texturePath) == 0)
        return;

    face.avalidable = false;
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

static void processTextureCoord(const Face &face, MapVertex *v, const glm::vec3 &tangent, const glm::vec3 &biTangent)
{
    for (int i = 0; i < 6; i++)
    {
        MapVertex &vertex = v[i];
        vertex.uv.x = glm::dot(tangent, vertex.position) / (face.textureWidth * face.scaleX);
        vertex.uv.y = glm::dot(biTangent, vertex.position) / (face.textureHeight * face.scaleY);
    }
}

static void processBack(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {1, 0, 0};
    static constexpr glm::vec3 biTangent = {0, 1, 0};
    static constexpr glm::vec3 normal = {0, 0, -1};

    v[0].position = min;
    v[1].position = {max.x, min.y, min.z};
    v[2].position = {min.x, max.y, min.z};
    v[3].position = {min.x, max.y, min.z};
    v[4].position = {max.x, min.y, min.z};
    v[5].position = {max.x, max.y, min.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}

static void processFront(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {1, 0, 0};
    static constexpr glm::vec3 biTangent = {0, 1, 0};
    static constexpr glm::vec3 normal = {0, 0, 1};

    v[0].position = max;
    v[1].position = {max.x, min.y, max.z};
    v[2].position = {min.x, max.y, max.z};
    v[3].position = {min.x, max.y, max.z};
    v[4].position = {max.x, min.y, max.z};
    v[5].position = {min.x, min.y, max.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}

static void processRight(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {0, 1, 0};
    static constexpr glm::vec3 biTangent = {0, 0, 1};
    static constexpr glm::vec3 normal = {1, 0, 0};

    v[0].position = max;
    v[1].position = {max.x, max.y, min.z};
    v[2].position = {max.x, min.y, max.z};
    v[3].position = {max.x, max.y, min.z};
    v[4].position = {max.x, min.y, min.z};
    v[5].position = {max.x, min.y, max.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}

static void processLeft(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {0, 1, 0};
    static constexpr glm::vec3 biTangent = {0, 0, 1};
    static constexpr glm::vec3 normal = {-1, 0, 0};

    v[0].position = min;
    v[1].position = {min.x, max.y, min.z};
    v[2].position = {min.x, min.y, max.z};
    v[3].position = {min.x, max.y, min.z};
    v[4].position = {min.x, max.y, max.z};
    v[5].position = {min.x, min.y, max.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}

static void processBottom(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {1, 0, 0};
    static constexpr glm::vec3 biTangent = {0, 0, 1};
    static constexpr glm::vec3 normal = {0, -1, 0};

    v[0].position = min;
    v[1].position = {min.x, min.y, max.z};
    v[2].position = {max.x, min.y, min.z};
    v[3].position = {min.x, min.y, max.z};
    v[4].position = {max.x, min.y, max.z};
    v[5].position = {max.x, min.y, min.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}

static void processTop(const glm::vec3 &min, const glm::vec3 &max, Face &face, MapVertex *v)
{
    static constexpr glm::vec3 tangent = {1, 0, 0};
    static constexpr glm::vec3 biTangent = {0, 0, 1};
    static constexpr glm::vec3 normal = {0, 1, 0};

    v[0].position = max;
    v[1].position = {min.x, max.y, max.z};
    v[2].position = {max.x, max.y, min.z};
    v[3].position = {min.x, max.y, max.z};
    v[4].position = {min.x, max.y, min.z};
    v[5].position = {max.x, max.y, min.z};

    for (int i = 0; i < 6; i++)
    {
        v[i].normal = normal;
    }
    processTextureCoord(face, v, tangent, biTangent);
}
