#include "Exception.hpp"

namespace  gage::utils
{
    Exception::Exception(std::string msg) :
        message{ std::move(msg) }
    {

    }
    const char* Exception::what() const noexcept
    {
        using namespace std::string_literals;

        buffer = "["s + typeid(const_cast<Exception&>(*this)).name() + "]"s;
        if(!message.empty()) {
            buffer += ": ";
            buffer += message;
        }
        return buffer.c_str();
    }
}