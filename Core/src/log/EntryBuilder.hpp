#pragma once

#include "Entry.hpp"
#include "Level.hpp"

namespace gage::log
{
    class IChannel;
    class EntryBuilder : private Entry
    {
    public:
        EntryBuilder(const char* file, const char* function, int line);
        EntryBuilder& note(std::string note);
        EntryBuilder& level(Level level);
        EntryBuilder& channel(IChannel* sink);
        EntryBuilder& trace(std::string str);
        EntryBuilder& debug(std::string str);
        EntryBuilder& info(std::string str);
        EntryBuilder& warn(std::string str);
        EntryBuilder& error(std::string str);
        EntryBuilder& fatal(std::string str);

        ~EntryBuilder();
    private:
        IChannel* sink_ = nullptr;
    };
}