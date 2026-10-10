#include "render.h"

#define cube "/Resources/3D_test/models/cube/scene.gltf"

std::vector<Model> vecModel = {};
std::vector<glm::vec3> vecPositions = {};

std::vector<AABB> vecAABB;
int attachListModel(const Model& i, glm::vec3 startingPos)
{
    vecModel.push_back(i);
    vecPositions.push_back(startingPos);

    return static_cast<int>(vecModel.size() - 1);
}

void attachAABB(size_t index)
{
    if (index >= vecModel.size()) return;

    vecModel[index].aabb.min =
        vecPositions[index] - glm::vec3(1.0f);

    vecModel[index].aabb.max =
        vecPositions[index] + glm::vec3(1.0f);
    

    
}

void loadModel(
    const std::string& parentDir,
    const std::string& modelPath,
    bool hitbox,
    glm::vec3 startingPos)
{
    std::string fullPath = parentDir + modelPath;


    Model model(fullPath.c_str());
    int index = attachListModel(model, startingPos);
    if (hitbox == true) {

        attachAABB(index);
    }

}

void addCube(
    const std::string& parentDir,
    glm::vec3 position
)
{
    loadModel(
        parentDir,
        cube,
        true,
        position
    );
}



void addAABB(glm::vec3 min, glm::vec3 max)
{
    AABB box;

    box.min = min;
    box.max = max;

    vecAABB.push_back(box);
}