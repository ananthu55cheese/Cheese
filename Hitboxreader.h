#ifndef HITBOX_H
#define HITBOX_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct HitboxData
{
    glm::vec3 min;
    glm::vec3 max;
};

std::vector<HitboxData> read(const std::string& fileName);

#endif