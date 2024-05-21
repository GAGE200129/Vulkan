#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initShaderModule(const std::vector<char> &code, vk::ShaderModule &module)
{
    vk::ShaderModuleCreateInfo ci;
    ci.setPCode((const uint32_t *)code.data()).setCodeSize(code.size());

    auto [result, shaderModule] = gData.device.createShaderModule(ci);
    if (result != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create shader module: {}", vk::to_string(result));
        return false;
    }
    module = shaderModule;
    return true;
}