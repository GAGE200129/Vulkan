#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>
#include <set>
#include <fstream>
#include <optional>
#include <iostream>
#include <thread>
#include <bitset>
#include <condition_variable>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <luajit-2.1/lua.hpp>

#include <BulletCollision/btBulletCollisionCommon.h>
#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT(x) 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

#include "Utils.hpp"