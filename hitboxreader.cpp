#include "Hitboxreader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::vector<HitboxData> read(const std::string& fileName)
{
    std::vector<HitboxData> hitboxes;

    std::ifstream file(fileName);

    if (!file.is_open())
    {
        std::cout
            << "Failed to open hitbox file: "
            << fileName
            << std::endl;

        return hitboxes;
    }

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);

        glm::vec3 point1;
        glm::vec3 point2;

        if (ss >>
            point1.x >>
            point1.y >>
            point1.z >>
            point2.x >>
            point2.y >>
            point2.z)
        {
            HitboxData hitbox;
            point1.y *= -1;
            point2.y *= -1;
            hitbox.min = glm::vec3(
                std::min(point1.x, point2.x),
                std::min(point1.z, point2.z),
                std::min(point1.y, point2.y)
            );

            hitbox.max = glm::vec3(
                std::max(point1.x, point2.x),
                std::max(point1.z, point2.z),
                std::max(point1.y, point2.y)
            );

            hitboxes.push_back(hitbox);
        }
        else
        {
            std::cout
                << "Invalid hitbox line: "
                << line
                << std::endl;
        }
    }

    file.close();

    return hitboxes;
}