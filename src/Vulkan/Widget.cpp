#include "pch.hpp"
#include "VulkanEngine.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

bool VulkanEngine::initWidget()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(gData.window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = gData.instance;
    init_info.PhysicalDevice = gData.physicalDevice.value();
    init_info.Device = gData.device;
    init_info.QueueFamily = gData.graphicsQueueFamily.value();
    init_info.Queue = gData.graphicQueue;
    init_info.DescriptorPool = gData.descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = gData.surfaceCapabilities.minImageCount;
    init_info.ImageCount = gData.swapchainImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn =
        [](VkResult result) {

        };
    ImGui_ImplVulkan_Init(&init_info, gData.renderPass);

    return true;
}

void VulkanEngine::cleanupWidget()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VulkanEngine::widgetBeginFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}
void VulkanEngine::widgetEndFrame(vk::CommandBuffer& cmd)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
}
