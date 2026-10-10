#include "Player.h"

player::player()
{
    pos = glm::vec3(0.0f, 0.0f, 2.0f);

    velocity = glm::vec3(0.0f);

    Orientation =
        glm::vec3(0.0f, 0.0f, -1.0f);

    Up =
        glm::vec3(0.0f, 1.0f, 0.0f);
    jumpHeight = 0.05f;
    speed = 0.02f;
    onGround = false;
}


void player::checkMovement(GLFWwindow* window)
{
    // Direction player is facing
    glm::vec3 forward =
        glm::vec3(
            Orientation.x,
            0.0f,
            Orientation.z
        );
    
    if (glm::length(forward) > 0.0f)
    {
        forward =
            glm::normalize(forward);
    }


    // Direction to the right
    glm::vec3 right =
        glm::normalize(
            glm::cross(forward, Up)
        );


    // Keyboard input
    glm::vec3 input =
        glm::vec3(0.0f);

    velocity.x *= 0.93f;
    velocity.z *= 0.93f;
    // W
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        input += forward;
    }


    // S
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        input -= forward;
    }


    // A
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        input -= right;
    }


    // D
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        input += right;
    }


    // Horizontal movement
    if (glm::length(input) > 0.0f)
    {
        input =
            glm::normalize(input);

        velocity +=
            input;
    }
   


    // Maximum horizontal speed
    glm::vec3 horizontalVelocity =
        glm::vec3(
            velocity.x,
            0.0f,
            velocity.z
        );

    if (glm::length(horizontalVelocity) > speed)
    {
        horizontalVelocity =
            glm::normalize(horizontalVelocity) *
            speed;

        velocity.x = horizontalVelocity.x;
        velocity.z = horizontalVelocity.z;
    }


    // ================================
    // JUMP
    // ================================

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS )
    {
        

        
        
        
        if (onGround)
        {
            velocity.y = jumpHeight;
            onGround = false;
        }
        
    }

    
}
    

    



void player::applyGravity()
{
    const float gravity = 0.0007f;

    if (!onGround)
    {
        velocity.y -= gravity;
    }
}