#include <pch.hpp>
#include "DefaultFileDriver.hpp"

#include "Entry.hpp"



namespace gage::log
{
    DefaultFileDriver::DefaultFileDriver(std::filesystem::path path)
    {
        std::filesystem::create_directories(path.parent_path());
        file_.open(path);
    }

    void DefaultFileDriver::submit(Entry &e)
    {
        file_ << e.note_ << "\n";
        if (e.trace_)
        {
            file_ << e.trace_->print() << "\n";
        }
        if (e.vk_result_)
        {
            file_ << "[Vulkan result]: " << string_VkResult((VkResult)e.vk_result_.value()) << "\n";
        }
    }
}