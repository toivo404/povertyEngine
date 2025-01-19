#pragma once
#include <vector>
#include <glm.hpp>

#include "Secs.h"
#include "systems/TransformSystem.h"


struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};

class PE_API PEPhysics
{
public:
    static AABB CalculateBoundingBox(const std::vector<glm::vec3>& vertices);
    static bool RaycastAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                            const AABB& aabb, const Transform& transform, float& tMin,
                            glm::vec3& intersectionPoint);
    static bool Raycast(
        const glm::vec3& rayOrigin, const glm::vec3& rayDirection, ::secs::World world,
        int transformComponent, int aabbComponent, float& distance, glm::vec3& intersectionPoint);
};
