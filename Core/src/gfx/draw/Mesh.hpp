#pragma once

#include "Drawable.hpp"

#include <tiny_gltf.h>

namespace gage::gfx::draw
{
    class Mesh : public Drawable
    {
    public:
        Mesh(Graphics& gfx, const tinygltf::Mesh& mesh)
        {

        }

        ~Mesh()
        {
            
        }
    };
}