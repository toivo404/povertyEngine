#include "PEPhysics.h"
#include <limits>
#include "systems/TransformSystem.h"
#include <limits>
#include "AssimpLoader.h"
#include "Secs.h"


AABB PEPhysics::CalculateBoundingBox(const std::vector<glm::vec3>& vertices)
{
    if (vertices.empty()) {
        throw std::invalid_argument("Mesh is empty.");
    }
    glm::vec3 min = vertices[0];
    glm::vec3 max = vertices[0];

    for (const auto& vertex : vertices) {
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }

    return {min, max};
}


bool PEPhysics::RaycastAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const AABB& aabb, const Transform& transform, float& tMin, glm::vec3& intersectionPoint)
{
    glm::mat4 invModel = glm::inverse(transform.model);
    glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localRayDirection = glm::normalize(glm::vec3(invModel * glm::vec4(rayDirection, 0.0f)));

    std::cout << "Local Ray Origin: " << localRayOrigin.x << ", " << localRayOrigin.y << ", " << localRayOrigin.z << "\n";
    std::cout << "Local Ray Direction: " << localRayDirection.x << ", " << localRayDirection.y << ", " << localRayDirection.z << "\n";

    glm::vec3 invDir = 1.0f / localRayDirection;
    glm::vec3 t1 = (aabb.min - localRayOrigin) * invDir;
    glm::vec3 t2 = (aabb.max - localRayOrigin) * invDir;

    glm::vec3 tMinVec = glm::min(t1, t2);
    glm::vec3 tMaxVec = glm::max(t1, t2);

    float tNear = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
    float tFar = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

    std::cout << "tNear: " << tNear << ", tFar: " << tFar << "\n";

    if (tNear > tFar || tFar < 0.0f) {
        std::cout << "No intersection.\n";
        return false; // No intersection
    }

    tMin = tNear;
    glm::vec3 localIntersectionPoint = localRayOrigin + localRayDirection * tMin;
    intersectionPoint = glm::vec3(transform.model * glm::vec4(localIntersectionPoint, 1.0f));

    std::cout << "Intersection Point: " << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z << "\n";

    return true;
}

bool PEPhysics::Raycast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, secs::World world, int transformComponentId, int aabbComponentId, PEPhysicsHitInfo& hitInfo)
{
    bool isHit = false;
    float minDist = std::numeric_limits<float>::max();
    for (secs::Entity entity : world.getAllEntities())
    {
        const auto aabb = world.tryGetComponent<AABB>(entity, aabbComponentId);
        const auto transform = world.tryGetComponent<Transform>(entity, transformComponentId);

        if (aabb == nullptr || transform == nullptr) continue;
        
        float dist;
        glm::vec3 point;
        const bool currentHit = RaycastAABB(rayOrigin, rayDirection, *aabb, *transform, dist, point);
        if (currentHit && dist < minDist)
        {
            minDist = dist;
            hitInfo.point = point;
            hitInfo.distance = dist;
            hitInfo.entity = entity; 
            isHit = true;
        }
    }
    return isHit;
}
