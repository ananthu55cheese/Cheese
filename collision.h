#ifndef collision_H
#define collison_H
#include "AABB.h"
#include "Player.h"
#include "Model.h"
#include <vector>

struct SATResult
{
    bool colliding = false;
    glm::vec3 normal = glm::vec3(0.0f);
    float penetration = 0.0f;
};

SATResult satAABB(
    const AABB& player,
    const AABB& obstacle
);

void resolveCollisions(
    player& Player,
    AABB& playerAABB,
    const std::vector<Model>& vecModel,
    const std::vector<AABB>& vecAABB
);

void updatePlayerbb(player& Player,
    AABB& playerAABB);

#endif 

