#pragma once

#include "IChannel.hpp"

namespace gage::log
{
    IChannel* get_default_channel();

    void init();
}

#define logger log::EntryBuilder{__FILE__, __FUNCTION__, __LINE__ }.channel(log::get_default_channel())