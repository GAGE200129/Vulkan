#include "EntryBuilder.hpp"

#include "IChannel.hpp"

namespace gage::log
{
    EntryBuilder::EntryBuilder(const char* file, const char* function, int line) :
        Entry {
            .level_ = Level::Error,
            .note_ = "",
            .file_ = file,
            .function_ = function,
            .line_ = line,
            .timestamp_ = std::chrono::system_clock::now(),
            .trace_ = std::nullopt
        }
    {

    }

    EntryBuilder::~EntryBuilder() 
    {
        if(sink_) 
        {
            if(level_ <= Level::Error)
            {
                trace_.emplace();
            }
            sink_->submit(*this);
        }
    }
    EntryBuilder &EntryBuilder::note(std::string note)
    {
        note_ = std::move(note);
        return *this;
    }
    EntryBuilder &EntryBuilder::level(Level level)
    {
        level_ = level;
        return *this;
    }
    EntryBuilder &EntryBuilder::channel(IChannel *sink)
    {
        sink_ = sink;
        return *this;
    }

    EntryBuilder& EntryBuilder::trace(std::string str)
    {
        level_ = Level::Trace;
        note_ = std::move(str);
        return *this;
    }
    EntryBuilder& EntryBuilder::debug(std::string str)
    {
        level_ = Level::Debug;
        note_ = std::move(str);
        return *this;
    }
    EntryBuilder& EntryBuilder::info(std::string str)
    {
        level_ = Level::Info;
        note_ = std::move(str);
        return *this;
    }
    EntryBuilder& EntryBuilder::warn(std::string str)
    {
        level_ = Level::Warn;
        note_ = std::move(str);
        return *this;
    }
    EntryBuilder& EntryBuilder::error(std::string str)
    {
        level_ = Level::Error;
        note_ = std::move(str);
        return *this;
    }
    EntryBuilder& EntryBuilder::fatal(std::string str)
    {
        level_ = Level::Fatal;
        note_ = std::move(str);
        return *this;
    }

}