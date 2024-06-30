#include "FileLoader.hpp"

#include "Exception.hpp"

#include <fstream>
#include <stb_image.h>
#include <cstring>
#include <cassert>

namespace gage::utils
{
    std::vector<char> file_path_to_binary(std::string file_path)
    {
        std::vector<char> buffer;

        try
        {
            std::ifstream f;
            ;
            f.open(file_path, std::ios::binary | std::ios::ate);

            buffer.resize(f.tellg());
            f.seekg(0, std::ios::beg);
            f.read(buffer.data(), buffer.size());
        }
        catch (std::exception &e)
        {
            throw FileLoaderException{"Error reading file: " + file_path};
            return {};
        }

        return buffer;
    }

    Image file_path_to_image(std::string file_path, int num_component)
    {
        assert(num_component >= 1 && num_component <= 4);
        assert(!file_path.empty());
        
        int width, height, bpp;
        //stbi_set_flip_vertically_on_load(1);
        stbi_uc *image = stbi_load(file_path.c_str(), &width, &height, &bpp, num_component);
        if (!image)
        {
            throw FileLoaderException{"Error image file: " + file_path};
            return {};
        }

        std::vector<char> result(width * height * 4);
        std::memcpy(result.data(), image, result.size());
        stbi_image_free(image);
        return {width, height, result};
    }
}