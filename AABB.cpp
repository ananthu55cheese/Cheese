#include "AABB.h"



bool AABB::checkcollision(const AABB& x, const AABB& y)
{
    return
        x.min.x <= y.max.x &&
        x.max.x >= y.min.x &&

        x.min.y <= y.max.y &&
        x.max.y >= y.min.y &&

        x.min.z <= y.max.z &&
        x.max.z >= y.min.z;
}