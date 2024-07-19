#include <pch.hpp>
#include "phys.hpp"

#include "Layers.hpp"
#include "BoardPhaseLayers.hpp"
#include "BPLayerInterface.hpp"
#include "ObjectVsBroadPhaseLayerFilter.hpp"
#include "ObjectLayerPairFilter.hpp"


namespace gage::phys
{
    using namespace JPH::literals;

    static void TraceImpl(const char *inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        //logger.trace(std::string(buffer));
    }
    // Callback for asserts, connect this to your own assert handler if you have one
    static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
    {
        // Print to the TTY
        std::stringstream ss;
        ss << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << "\n";
        //logger.error(ss.str());
        assert(false);
        return true;
    };

    void init()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

    }

   
    void shutdown()
    {

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}