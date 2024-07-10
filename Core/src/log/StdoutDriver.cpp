#include <pch.hpp>
#include "StdoutDriver.hpp"

#include "Entry.hpp"




namespace gage::log
{
    void StdoutDriver::submit(log::Entry &e)
    {
        std::cout << e.note_ << "\n";
        if (e.trace_)
            std::cout << e.trace_->print() << "\n";
        if (e.vk_result_)
            std::cout << "[Vulkan result]: " << string_VkResult((VkResult)e.vk_result_.value()) << "\n";
    }
}