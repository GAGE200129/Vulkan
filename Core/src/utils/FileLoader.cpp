#include "FileLoader.hpp"

#include "Exception.hpp"

#include <fstream>

namespace gage::utils
{
    std::vector<char> file_path_to_binary(std::string file_path)
    {
        std::vector<char> buffer;

        try
        {
            std::ifstream f;;
            f.open(file_path, std::ios::binary | std::ios::ate);
            
            buffer.resize(f.tellg());
            f.seekg(0, std::ios::beg);
            f.read(buffer.data(), buffer.size());

        } catch(std::exception& e)
        {
            throw FileLoaderException{"Error reading file: " + file_path};
            return {};
        }

        return buffer;
    }
}