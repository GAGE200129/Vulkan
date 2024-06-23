#include <stdio.h>

#include <iostream>

#include <Core/src/log/EntryBuilder.hpp>
#include <Core/src/log/IChannel.hpp>
#include <Core/src/log/DefaultChannel.hpp>
#include <Core/src/log/IDriver.hpp>
#include <Core/src/log/IPolicy.hpp>
#include <Core/src/log/ServerityLevelPolicy.hpp>
#include <Core/src/log/Log.hpp>

using namespace gage;

void init()
{
    log::init();
}

void testing3()
{
    logger.fatal("Fatal");
    logger.warn("Warn");
    logger.error("Error");
}

void testing2()
{
    testing3();
}

void testing()
{
    testing2();
}

int main()
{
    init();
    testing();
    return 0;
}