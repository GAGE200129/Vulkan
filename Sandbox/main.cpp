#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>

#include <thread>

using namespace gage;
using namespace std::chrono_literals;

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

    auto win = std::make_shared<win::Window>(320, 240, "Hello world");
    auto win2 = std::make_shared<win::Window>(320, 240, "Hello world2");

    while(!win->is_closing() && !win2->is_closing())
    {
        
        std::this_thread::sleep_for(100ms);
    
        win::update();
    }

    shutdown();
    return 0;
}