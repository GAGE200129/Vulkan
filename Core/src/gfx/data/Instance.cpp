#include <pch.hpp>
#include "Instance.hpp"

#include "../Exception.hpp"
#include "../Graphics.hpp"

using namespace std::string_literals;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *)
{
    using namespace gage;
    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        gfx::log().info("{}", p_callback_data->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        gfx::log().info("{}", p_callback_data->pMessage);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        gfx::log().warn("{}", p_callback_data->pMessage);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        utils::StackTrace stack_trace;
        gfx::log().error("{}\n{}", p_callback_data->pMessage, stack_trace.print());

        break;
    }

    default:
        break;
    }

    return VK_FALSE;
}

namespace gage::gfx::data
{
    Instance::Instance(const std::string &app_name, GLFWwindow* window)
    {
        vkb::InstanceBuilder builder;
        // make the vulkan instance, with basic debug features
        auto vkb_instance_result = builder.set_app_name(app_name.c_str())
                                       .request_validation_layers(true)
                                       .set_debug_callback(debugCallback)
                                       .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                                       .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                                       .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                                       .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                                       .set_debug_callback_user_data_pointer(this)
                                       .enable_extensions(Graphics::ENABLED_INSTANCE_EXTENSIONS.size(), Graphics::ENABLED_INSTANCE_EXTENSIONS.data())
                                       .require_api_version(1, 3, 0)
                                       .build();
        vkb_check(vkb_instance_result, "Failed to create vulkan instance !"s);

        // grab the instance
        vkb_instance = vkb_instance_result.value();
        instance = vkb_instance.instance;
        debug_messenger = vkb_instance.debug_messenger;

        vk_check(glfwCreateWindowSurface(instance, window, nullptr, &surface));
        
    }

    Instance::~Instance()
    {
        vkb::destroy_debug_utils_messenger(instance, debug_messenger);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}