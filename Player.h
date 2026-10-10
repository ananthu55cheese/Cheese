#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class player
{
public:

    player();

    glm::vec3 pos;
    glm::vec3 velocity;

    glm::vec3 Orientation;
    glm::vec3 Up;
    bool onGround;
    float speed;
    float jumpHeight;
    void checkMovement(GLFWwindow* window);
    void applyGravity();
};