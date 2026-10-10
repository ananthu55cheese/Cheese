#include <windows.h>
#include "AABB.h"
#include "Model.h"
#include <vector>
#include <iostream>
#include <cfloat>
#include <algorithm>
#ifndef Render_H
#define Render_H
extern std::vector<Model> vecModel;
extern std::vector<glm::vec3> vecPositions;
extern std::vector<AABB> vecAABB;
int attachListModel(const Model& i, glm::vec3 startingPos);
void attachAABB(size_t index);
void loadModel(
    const std::string& parentDir,
    const std::string& modelPath,
    bool hitbox,
    glm::vec3 startingPos);
void addCube(
    const std::string& parentDir,
    glm::vec3 position
);

void addAABB(glm::vec3 min, glm::vec3 max);



#endif 

