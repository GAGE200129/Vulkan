#pragma once

#include "IChannel.hpp"
#include "EntryBuilder.hpp"

namespace gage::log
{
    IChannel* get_singeton_channel();

    void init();
}

#define logger log::EntryBuilder{__FILE__, __FUNCTION__, __LINE__ }.channel(log::get_singeton_channel())