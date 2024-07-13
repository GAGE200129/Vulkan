#include <pch.hpp>
#include "Cvar.hpp"

namespace gage::utils
{
    std::vector<Cvar*> Cvar::cvars; 

    Cvar::Cvar(std::string name, Data data) :
        name(std::move(name)),
        data(data)
    {
        create_cvar(*this);
    }
}