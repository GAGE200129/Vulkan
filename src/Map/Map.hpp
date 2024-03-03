#pragma once
#include <vector>

#include "Vulkan/VulkanEngine.hpp"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"

// Static class that handles currently loaded map
class Map
{
public:
  static void load(const std::string &filePath);
  static void save(const std::string &fileName);
private:
  static void processMapPiece(const std::string& line);
private:
  //static map mesh
  static VulkanBuffer mPositionBuffer, mNormalBuffer, mUvBuffer, mIndexBuffer;
  static btCollisionObject* mMapCollisionObject;
  static btBvhTriangleMeshShape* mMapCollisionShape;
};