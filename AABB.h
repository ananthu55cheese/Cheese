#ifndef AABB_CLASS
#define AABB_CLASS
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
class AABB {
	
public:
	glm::vec3 min;
	glm::vec3 max; 

	static bool checkcollision(const AABB& x,const AABB& y);

};

#endif // !AABB_CLASS

