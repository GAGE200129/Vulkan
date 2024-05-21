#include "pch.hpp"
#include "FileLoader.hpp"

bool FileLoader::filePathToVectorOfChar(const std::string& filePath, std::vector<char>& v)
{
    std::ifstream f(filePath, std::ios::ate | std::ios::binary);

    if (!f.is_open())
    {
        spdlog::error("failed to open file: {}", filePath);
        return false;
    }

    size_t fileSize = (size_t)f.tellg();
    v.resize(fileSize);
    f.seekg(0);
    f.read(v.data(), fileSize);
    f.close();

    return true;
}
