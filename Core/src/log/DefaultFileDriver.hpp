#pragma once

#include "IDriver.hpp"



namespace gage::log
{
    class DefaultFileDriver : public IDriver
    {
    public:
        DefaultFileDriver(std::filesystem::path path);
        void submit(Entry& ) override; 
    private:
        std::ofstream file_;
    };
};