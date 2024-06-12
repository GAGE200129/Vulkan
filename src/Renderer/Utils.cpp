#include "pch.hpp"
#include "Renderer.hpp"


void Renderer::deletionQueueFlush(DeletionQueue &queue)
{
    // reverse iterate the deletion queue to execute all the functions
    for (auto it = queue.rbegin(); it != queue.rend(); it++)
    {
        (*it)(); // call functors
    }

    queue.clear();
}


std::optional<vk::ShaderModule> Renderer::loadShaderModule(const std::string& filePath)
{
    // open the file. With cursor at the end
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return {};
    }

     // find what the size of the file is by looking up the location of the cursor
    // because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    // spirv expects the buffer to be on uint32, so make sure to reserve a int
    // vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

      // put file cursor at beginning
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    vk::ShaderModule shaderModule;

    auto result = gData.device.createShaderModule(createInfo);

    if(result.result != vk::Result::eSuccess)
    {
        gLogger->warn("Failed to load shadermodule: {} | error: {}", filePath, vk::to_string(result.result));
        return {};
    }
    
    return std::make_optional(result.value);
}