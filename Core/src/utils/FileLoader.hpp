#pragma once

#include <vector>
#include <string>

namespace gage::utils
{
    struct Image
    {   
        int width{}, height{};
        std::vector<char> data{};
    };
    std::vector<char> file_path_to_binary(std::string file_path);
    Image file_path_to_image(std::string file_path, int num_component);
}