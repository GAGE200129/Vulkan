#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>

using namespace gage;

void init()
{
    log::init();
    win::init();
}

void shutdown()
{

    win::shutdown();
}



int main()
{
    init();
    
    auto win = std::make_shared<win::Window>(1600, 900, "Hello world");

    shutdown();
    return 0;
}