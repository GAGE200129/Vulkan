#pragma once


#include <string>
#include <vector>
#include "Face.hpp"


class Polygon
{
public:

public:
  std::vector<std::tuple<glm::dvec3, glm::dvec2>> vertices;
};

class Map
{
public:
  void load(const std::string& filePath);
private:
  std::vector<Face> mFaces;
  std::vector<Polygon> mPolygons;
};