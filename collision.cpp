#include "Collision.h"

#include <glm/glm.hpp>
#include <cmath>
#include <limits>

// =====================================================
// SAT AABB
// =====================================================

SATResult satAABB(
    const AABB& player,
    const AABB& obstacle)
{
    glm::vec3 playerHalf =
        (player.max - player.min) * 0.5f;

    glm::vec3 obstacleHalf =
        (obstacle.max - obstacle.min) * 0.5f;

    glm::vec3 playerCenter =
        (player.min + player.max) * 0.5f;

    glm::vec3 obstacleCenter =
        (obstacle.min + obstacle.max) * 0.5f;

    glm::vec3 delta =
        playerCenter - obstacleCenter;

    // SAT overlap on each axis
    glm::vec3 overlap;

    overlap.x =
        playerHalf.x +
        obstacleHalf.x -
        std::abs(delta.x);

    overlap.y =
        playerHalf.y +
        obstacleHalf.y -
        std::abs(delta.y);

    overlap.z =
        playerHalf.z +
        obstacleHalf.z -
        std::abs(delta.z);

    // No collision
    if (overlap.x <= 0.0f ||
        overlap.y <= 0.0f ||
        overlap.z <= 0.0f)
    {
        return {};
    }

    // Find axis with smallest penetration
    int axis = 0;

    if (overlap.y < overlap.x &&
        overlap.y <= overlap.z)
    {
        axis = 1;
    }
    else if (overlap.z < overlap.x &&
        overlap.z < overlap.y)
    {
        axis = 2;
    }

    SATResult result;

    result.colliding = true;
    result.penetration = overlap[axis];

    // Normal points from obstacle toward player
    if (delta[axis] >= 0.0f)
    {
        result.normal[axis] = 1.0f;
    }
    else
    {
        result.normal[axis] = -1.0f;
    }

    return result;
}


// =====================================================
// RESOLVE COLLISIONS
// =====================================================

void resolveCollisions(
    player& Player,
    AABB& playerAABB,
    const std::vector<Model>& vecModel,
    const std::vector<AABB>& vecAABB)
{
    for (int iteration = 0; iteration < 4; ++iteration)
    {
        bool resolvedCollision = false;

        // ---------------------------------------------
        // Update player AABB
        // ---------------------------------------------

        updatePlayerbb(Player, playerAABB);

        // =====================================================
        // MODEL COLLISIONS
        // =====================================================

        for (const Model& model : vecModel)
        {
            SATResult hit =
                satAABB(
                    playerAABB,
                    model.aabb
                );

            if (!hit.colliding)
                continue;

            resolvedCollision = true;

            // Push player out
            Player.pos +=
                hit.normal * hit.penetration;


            // ---------------------------------------------
            // Ground
            // ---------------------------------------------

            if (hit.normal.y > 0.5f)
            {
                Player.onGround = true;

                if (Player.velocity.y < 0.0f)
                {
                    Player.velocity.y = 0.0f;
                }
            }


            // ---------------------------------------------
            // Ceiling
            // ---------------------------------------------

            else if (hit.normal.y < -0.5f)
            {
                if (Player.velocity.y > 0.0f)
                {
                    Player.velocity.y = 0.0f;
                }
            }


            // ---------------------------------------------
            // Wall
            // ---------------------------------------------

            else
            {
                float velocityIntoWall =
                    glm::dot(
                        Player.velocity,
                        hit.normal
                    );

                if (velocityIntoWall < 0.0f)
                {
                    Player.velocity -=
                        hit.normal * velocityIntoWall;
                }
            }


            // Update AABB
            updatePlayerbb(Player, playerAABB);
        }


        // =====================================================
        // STANDALONE AABB COLLISIONS
        // =====================================================

        for (const AABB& box : vecAABB)
        {
            SATResult hit =
                satAABB(
                    playerAABB,
                    box
                );

            if (!hit.colliding)
                continue;

            resolvedCollision = true;

            // Push player out
            Player.pos +=
                hit.normal * hit.penetration;


            // ---------------------------------------------
            // Ground
            // ---------------------------------------------

            if (hit.normal.y > 0.5f)
            {
                Player.onGround = true;

                if (Player.velocity.y < 0.0f)
                {
                    Player.velocity.y = 0.0f;
                }
            }


            // ---------------------------------------------
            // Ceiling
            // ---------------------------------------------

            else if (hit.normal.y < -0.5f)
            {
                if (Player.velocity.y > 0.0f)
                {
                    Player.velocity.y = 0.0f;
                }
            }


            // ---------------------------------------------
            // Wall
            // ---------------------------------------------

            else
            {
                float velocityIntoWall =
                    glm::dot(
                        Player.velocity,
                        hit.normal
                    );

                if (velocityIntoWall < 0.0f)
                {
                    Player.velocity -=
                        hit.normal * velocityIntoWall;
                }
            }


            updatePlayerbb(Player, playerAABB);
            
        }


        // No more collisions
        if (!resolvedCollision)
            break;
    }
}

void updatePlayerbb(player& Player,
    AABB& playerAABB) {
    

    playerAABB.min = glm::vec3(
        Player.pos.x - 0.3f,
        Player.pos.y - 3.6f,
        Player.pos.z - 0.3f
    );

    playerAABB.max = glm::vec3(
        Player.pos.x + 0.3f,
        Player.pos.y,
        Player.pos.z + 0.3f
    );

}