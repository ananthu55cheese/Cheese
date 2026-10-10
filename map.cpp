#include "map.h"
#include <filesystem>
namespace fs = std::filesystem;
#include "Hitboxreader.h"
//std::string parentDir = fs::current_path().string();
std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
void generateMap() {
    
    

    
    std::vector<HitboxData> hitboxes =
        read("level1.hitbox");;

    for (const HitboxData& hitbox : hitboxes)
    {
        addAABB(
            hitbox.min,
            hitbox.max
        );
    }
   
    
    
    
    
    
    loadModel(parentDir, "/Resources/3D_test/models/level 1/level1.gltf",
        false,
        glm::vec3(0.0f, 0.0f, 0.0f)
    );
   

    addCube(parentDir, glm::vec3(0.0f, 10.0f, 0.0f));

    
        


   
}