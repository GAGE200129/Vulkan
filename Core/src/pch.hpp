
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>

#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <vulkan/vk_enum_string_helper.h>

#include <stb_image.h>
#include <tiny_gltf.h>

#include <cstring>
#include <cstddef>
#include <cstdint>

#include <vector>
#include <tuple>
#include <sstream>
#include <string>
#include <functional>
#include <stack>
#include <optional>
#include <mutex>
#include <span>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <exception>
#include <cassert>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>



