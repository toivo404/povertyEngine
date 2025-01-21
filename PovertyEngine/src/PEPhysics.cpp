#include "PEPhysics.h"
#include <limits>
#include "systems/TransformSystem.h"
#include <limits>
#include "AssimpLoader.h"
#include "Engine.h"
#include "Secs.h"


void AABB::AABBView(glm::vec3& cameraPosition, glm::vec3& cameraTarget, const glm::vec3& offsetDirection, float zoomMultiplier)
{
    // Calculate the center of the AABB
    glm::vec3 center = (min + max) * 0.5f;

    // Calculate the size of the AABB
    glm::vec3 size = max - min;
    float radius = glm::length(size) * 0.5f; // Approximate radius of a bounding sphere

    // Set the desired angle (45 degrees downwards)
    float elevationAngle = glm::radians(45.0f); // Downward angle in radians
    float azimuthAngle = glm::radians(45.0f);   // Side angle (optional)

    // Calculate camera position in spherical coordinates
    float distance = radius / std::sin(elevationAngle); // Ensure AABB fits in view
    float x = distance * std::cos(elevationAngle) * std::cos(azimuthAngle);
    float z = distance * std::cos(elevationAngle) * std::sin(azimuthAngle);
    float y = distance * std::sin(elevationAngle);

    // Set camera position
    cameraPosition = center + glm::vec3(x, y, z);

    // Set camera target to look at the center of the AABB
    cameraTarget = center;
}

void AABB::Encapsulate(glm::vec3 vec)
{
    min.x = std::min(min.x, vec.x);
    min.y = std::min(min.y, vec.y);
    min.z = std::min(min.z, vec.z);
    max.x = std::max(max.x, vec.x);
    max.y = std::max(max.y, vec.y);
    max.z = std::max(max.z, vec.z);
}

void AABB::DebugDraw(glm::vec3 color, const Transform& transform)
{
    // Calculate the transformed AABB corners
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(max.x, max.y, max.z),
    };

    glm::mat4 modelMatrix = transform.model;
    for (int i = 0; i < 8; ++i)
    {
        corners[i] = glm::vec3(modelMatrix * glm::vec4(corners[i], 1.0f));
    }

    // Draw edges of the AABB
    static const int edges[12][2] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0}, // Bottom face
        {4, 5}, {5, 7}, {7, 6}, {6, 4}, // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7} // Vertical edges
    };

    for (const auto& edge : edges)
    {
        Engine::DebugDrawLine(corners[edge[0]], corners[edge[1]], color, 0.1f);
    }
}

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
