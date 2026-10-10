//------- Ignore this ----------
#include <filesystem>
namespace fs = std::filesystem;
//------------------------------
#include <windows.h>
#include "AABB.h"
#include "Model.h"
#include <vector>
#include <iostream>
#include "render.h"
#define other "/Resources/3D_test/models/weird/scene.gltf"
#define map "/Resources/3D_test/models/map/scene.gltf"
#include "map.h"
#define cube "/Resources/3D_test/models/cube/scene.gltf"
#define bunny "/Resources/3D_test/models/bunny/scene.gltf"
#include "Player.h"
#include <limits>
#include <cmath>
#include "collision.h"


// SAT for two axis-aligned bounding boxes.
// normal points FROM the obstacle TOWARD the player.


const unsigned int width = 1300;
const unsigned int height = 850;

glm::vec3 spawnPoint = glm::vec3(
    0.0f,
    15.0f,
    0.0f
);


void checkexit(GLFWwindow* window) {
    // Check if the Escape key is currently being pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        // Flag the window to close safely
        glfwSetWindowShouldClose(window, true);
    }
}


class object {
public:
    glm::vec3 pos;
    size_t index;

    object(
        glm::vec3 startingPos,
        const std::string& parentDir,
        const std::string& modelPath)
    {
        std::string fullPath = parentDir + modelPath;

        Model model(fullPath.c_str());

        pos = startingPos;
        
        index = attachListModel(model, pos);
    }

    void update()
    {
        vecPositions[index] = pos;
    }
};


int main()
{
    
    

    
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );


    

    GLFWwindow* window =
        glfwCreateWindow(
            width,
            height,
            "test",
            NULL,
            NULL
        );

    if (window == NULL)
    {
        std::cout
            << "Failed to create GLFW window"
            << std::endl;

        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);


    gladLoadGL();

    glViewport(
        0,
        0,
        width,
        height
    );


 
    Shader shaderProgram(
        "default.vert",
        "default.frag"
    );



    glm::vec4 lightColor =
        glm::vec4(
            1.0f,
            1.0f,
            1.0f,
            1.0f
        );

    glm::vec3 lightPos =
        glm::vec3(
            0.5f,
            0.5f,
            0.5f
        );

    shaderProgram.Activate();

    glUniform4f(
        glGetUniformLocation(
            shaderProgram.ID,
            "lightColor"
        ),
        lightColor.x,
        lightColor.y,
        lightColor.z,
        lightColor.w
    );

    glUniform3f(
        glGetUniformLocation(
            shaderProgram.ID,
            "lightPos"
        ),
        lightPos.x,
        lightPos.y,
        lightPos.z
    );


 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    

    Camera camera(
        width,
        height,
        glm::vec3(
            0.0f,
            0.0f,
            0.0f
        )
    );


    // when debugging use this:
    std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
    //release use this:
    //std::string parentDir = fs::current_path().string();
    //note this is only for vscode
    player Player;

    Player.pos = spawnPoint;
  

    
    
        
   
    generateMap();
   
    AABB playerAABB;
    
    while (!glfwWindowShouldClose(window))
    {
        
       
        
        glClearColor(
            0.07f,
            0.13f,
            0.17f,
            1.0f
        );
        
        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );
        
        
        

        camera.Inputs(window);


        
        Player.Orientation =
            camera.Orientation;

        if (Player.pos.y < -20.0f) {

            Player.pos = spawnPoint;

        }
        
        
        char title[128];

        sprintf_s(
            title,
            "X: %.2f | Y: %.2f | Z: %.2f",
            Player.pos.x,
            Player.pos.y,
            Player.pos.z
        );

        glfwSetWindowTitle(window, title);

        
        Player.applyGravity();

        Player.checkMovement(window);
        
        glm::vec3 nextPosition =
            Player.pos + Player.velocity;

        playerAABB.min =
            nextPosition - glm::vec3(0.3f);

        playerAABB.max =
            nextPosition + glm::vec3(0.3f);
        

        AABB collisionChecker;

        
        Player.pos = nextPosition;
        Player.onGround = false;
        Player.onGround = false;

        resolveCollisions(
            Player,
            playerAABB,
            vecModel,
            vecAABB
        );

        
        camera.Position =
            Player.pos;


        
        camera.updateMatrix(
            45.0f,
            0.1f,
            100.0f
        );
        
           
        checkexit(window);
      
        for (size_t i = 0;
            i < vecModel.size();
            i++)
        {
            glm::mat4 modelMatrix =
                glm::mat4(1.0f);


            
            modelMatrix =
                glm::translate(
                    modelMatrix,
                    vecPositions[i]
                );


            
            glUniformMatrix4fv(
                glGetUniformLocation(
                    shaderProgram.ID,
                    "model"
                ),
                1,
                GL_FALSE,
                glm::value_ptr(
                    modelMatrix
                )
            );


            // Draw model
            vecModel[i].Draw(
                shaderProgram,
                camera,
                modelMatrix
            );
        }



        glfwSwapBuffers(window);

        glfwPollEvents();
    }


   

    shaderProgram.Delete();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}