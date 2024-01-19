#pragma once
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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <lua5.4/lua.hpp>

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "Utils.hpp"