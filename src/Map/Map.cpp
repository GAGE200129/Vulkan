#include "pch.hpp"
#include "Map.hpp"

void Map::load(const std::string &filePath)
{
  std::ifstream f(filePath);
  if (!f.is_open())
    throw std::runtime_error("Failed to load map: " + filePath);

  std::string line;
  while (std::getline(f, line))
  {
    if(line.find("#") == line.end())
      break;
    processMapPiece(line);
  }
}

void Map::processMapPiece(const std::string &line)
{
  std::vector<std::string> tokens;
  std::string token;
  std::stringstream lineSS(line);
  while (lineSS >> token)
  {
    tokens.push_back(token);
  }
}