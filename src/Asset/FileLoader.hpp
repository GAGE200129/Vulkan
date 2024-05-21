#pragma once

#include <vector>

namespace FileLoader
{
    bool filePathToVectorOfChar(const std::string& filePath, std::vector<char>& v);
}