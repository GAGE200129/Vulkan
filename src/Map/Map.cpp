#include "pch.hpp"
#include "Map.hpp"

void Map::load(const std::string &filePath)
{
  mFaces.emplace_back(glm::ivec3{-48, -176, 32}, glm::ivec3{-48, -175, 32}, glm::ivec3{-48, -176, 33});
  mFaces.emplace_back(glm::ivec3{-48, -176, 32}, glm::ivec3{-48, -176, 33}, glm::ivec3{-47, -176, 32});
  mFaces.emplace_back(glm::ivec3{-48, -176, 32}, glm::ivec3{-47, -176, 32}, glm::ivec3{-48, -175, 32});
  mFaces.emplace_back(glm::ivec3{80, -32, 48}, glm::ivec3{80, -31, 48}, glm::ivec3{81, -32, 48});
  mFaces.emplace_back(glm::ivec3{80, -32, 48}, glm::ivec3{81, -32, 48}, glm::ivec3{80, -32, 49});
  mFaces.emplace_back(glm::ivec3{80, -32, 48}, glm::ivec3{80, -32, 49}, glm::ivec3{80, -31, 48});


  mPolygons.resize(mFaces.size());
  for (int i = 0; i < mFaces.size() - 2; i++)
    for (int j = i + 1; j < mFaces.size() - 1; j++)
      for (int k = j + 1; k < mFaces.size(); k++)
      {
        const Face &faceI = mFaces.at(i);
        const Face &faceJ = mFaces.at(j);
        const Face &faceK = mFaces.at(k);
        const glm::dvec3 &n1 = faceI.mNormal;
        const glm::dvec3 &n2 = faceJ.mNormal;
        const glm::dvec3 &n3 = faceK.mNormal;
        double d1 = faceI.mDistance;
        double d2 = faceJ.mDistance;
        double d3 = faceK.mDistance;

        double denom = glm::dot(n1, glm::cross(n2, n3));
        if (denom != 0.0)
        {
          glm::dvec3 p = -d1 * glm::cross(n2, n3) - d2 * glm::cross(n3, n1) - d3 * glm::cross(n1, n2);
          p /= denom;
          bool illegal = false;
          for (int x = 0; x < mFaces.size(); x++)
          {
            const Face &face = mFaces.at(i);
            const glm::dvec3& n = face.mNormal;
            double d = face.mDistance;

            if(glm::dot(n, p) + d > 0.0)
              illegal = true;
          }
          if(!illegal) continue;

          mPolygons[i].vertices.push_back(std::make_tuple(p, glm::dvec2{}));
          mPolygons[j].vertices.push_back(std::make_tuple(p, glm::dvec2{}));
          mPolygons[k].vertices.push_back(std::make_tuple(p, glm::dvec2{}));
        }
      }
}
