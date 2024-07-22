#include <pch.hpp>
#include "mem.hpp"

#include <cstdint>
#include <exception>
#include <cstdlib>


namespace gage
{
    static uint32_t allocated_bytes{}; 
    uint32_t get_allocated_bytes()
    {
        return allocated_bytes;
    }
}

 
void * operator new(std::size_t n)
{
    if (n == 0)
        n++; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(n))
    {
        gage::allocated_bytes += n;
        return ptr;
    }
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}




void operator delete(void * p)
{
    std::free(p);
}
