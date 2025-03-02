#pragma once
#include <vector>
#include <glm.hpp>
#include "Secs.h"
#include "systems/TransformSystem.h"
#include <core.h>

struct PE_API AABB
{
    glm::vec3 min;
    glm::vec3 max;

    void AABBView(glm::vec3& cameraPosition, glm::vec3& cameraTarget, const glm::vec3& offsetDirection, float zoomMultiplier = 1.0f);

    void Encapsulate(glm::vec3 vec);
    
    void DebugDraw(glm::vec3 color, const Transform& transform);
};

struct PEPhysicsHitInfo
{
    secs::Entity entity;
    glm::vec3 point;
    float distance;
};

class PE_API PEPhysics
{
public:
    static AABB CalculateBoundingBox(const std::vector<glm::vec3>& vertices);
    static bool RaycastAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                            const AABB& aabb, const Transform& transform, float& tMin,
                            glm::vec3& intersectionPoint);
    static bool CheckAABBOverlap(const AABB& aabb1, const Transform& transform1,
                                 const AABB& aabb2, const Transform& transform2);
    static bool Raycast(
        const glm::vec3& rayOrigin, const glm::vec3& rayDirection, ::secs::World& world, PEPhysicsHitInfo& hitInfo);
    
    static bool RaycastMesh(const glm::vec3& rayOrigin,
                            const glm::vec3& rayDirection,
                            const std::vector<glm::vec3>& vertices,
                            const std::vector<unsigned int>& indices,
                            const Transform& transform,
                            float& tMin,
                            glm::vec3& intersectionPoint);
};