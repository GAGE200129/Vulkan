#pragma once

#include <exception>
#include <string>

namespace gage::utils
{
    class Exception : public std::exception
    {
    public:
        Exception() = default;
        Exception(std::string msg);
        const char* what() const noexcept override;
    private:
        std::string message;
        mutable std::string buffer;
    };
}