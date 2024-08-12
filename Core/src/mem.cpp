#include <pch.hpp>
#include "mem.hpp"

#include <cstdint>
#include <exception>
#include <cstdlib>


namespace gage
{
    static size_t allocated_bytes{}; 

    size_t get_allocated_bytes()
    {
        return allocated_bytes;
    }
}

 
void * operator new(std::size_t n)
{
    if (n == 0)
        n++;
 
    if (void *ptr =std::malloc(n))
    {
        gage::allocated_bytes += n;
        return ptr;
    }
 
    throw std::bad_alloc{};
}




void operator delete(void * p, std::size_t n)
{
    gage::allocated_bytes -= n;
    std::free(p);
}
