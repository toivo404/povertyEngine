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

    glm::vec3 invDir = 1.0f / localRayDirection;
    glm::vec3 t1 = (aabb.min - localRayOrigin) * invDir;
    glm::vec3 t2 = (aabb.max - localRayOrigin) * invDir;

    glm::vec3 tMinVec = glm::min(t1, t2);
    glm::vec3 tMaxVec = glm::max(t1, t2);

    float tNear = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
    float tFar = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);


    if (tNear > tFar || tFar < 0.0f) {
        return false; // No intersection
    }

    tMin = tNear;
    glm::vec3 localIntersectionPoint = localRayOrigin + localRayDirection * tMin;
    intersectionPoint = glm::vec3(transform.model * glm::vec4(localIntersectionPoint, 1.0f));


    return true;
}

bool PEPhysics::Raycast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, secs::World& world, PEPhysicsHitInfo& hitInfo)
{
    bool isHit = false;
    float minDist = std::numeric_limits<float>::max();
    secs::queryChunks<Transform, AABB>(world, [&](
        const std::vector<secs::Entity>& ents,
        const Transform* transform,
        const AABB* aabb,
        const size_t count
        )
    {
        for (size_t i = 0; i < count; ++i)
        {
            float dist;
            glm::vec3 point;
            const bool currentHit = RaycastAABB(rayOrigin, rayDirection, aabb[i], transform[i], dist, point);
            if (currentHit && dist < minDist)
            {
                minDist = dist;
                hitInfo.point = point;
                hitInfo.distance = dist;
                hitInfo.entity = ents[i];
                isHit = true;
            }
        }
    });
  
    return isHit;
    
}



namespace
{
    // -----------------------------------------------------------------------------
    // Möller–Trumbore algorithm to intersect a ray with a single triangle.
    // Returns true if the ray intersects and sets outT to the parametric distance
    // along the ray. outU and outV are the barycentric coordinates (optional).
    // -----------------------------------------------------------------------------
    bool MollerTrumboreIntersection(const glm::vec3& origin,
                                    const glm::vec3& direction,
                                    const glm::vec3& v0,
                                    const glm::vec3& v1,
                                    const glm::vec3& v2,
                                    float& outT,
                                    float& outU,
                                    float& outV)
    {
        const float EPSILON = 1e-6f;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        // pvec is perpendicular to edge2 and direction
        glm::vec3 pvec = glm::cross(direction, edge2);
        float det = glm::dot(edge1, pvec);

        // If det is near zero, the ray lies in the plane of the triangle or is parallel
        if (fabs(det) < EPSILON)
            return false;

        float invDet = 1.0f / det;

        // Distance from v0 to ray origin
        glm::vec3 tvec = origin - v0;
        float u = glm::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f)
            return false;

        // qvec is perpendicular to edge1 and tvec
        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = glm::dot(direction, qvec) * invDet;
        if (v < 0.0f || (u + v) > 1.0f)
            return false;

        float t = glm::dot(edge2, qvec) * invDet;
        if (t < 0.0f)
            return false;

        outT = t;
        outU = u;
        outV = v;
        return true;
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
bool PEPhysics::RaycastMesh(const glm::vec3& rayOrigin,
                            const glm::vec3& rayDirection,
                            const std::vector<glm::vec3>& vertices,
                            const std::vector<unsigned int>& indices,
                            const Transform& transform,
                            float& tMin,
                            glm::vec3& intersectionPoint)
{
    // Early out: sanity checks
    if (vertices.empty() || indices.size() < 3)
    {
        return false;
    }

    // Transform ray to local space
    glm::mat4 invModel = glm::inverse(transform.model);
    glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    // For consistent 'distance', we typically keep direction normalized
    glm::vec3 localRayDirection = glm::normalize(glm::vec3(invModel * glm::vec4(rayDirection, 0.0f)));

    bool hit = false;
    float closestT = std::numeric_limits<float>::max();

    // We’ll store a temporary intersection point in local space for comparison
    glm::vec3 bestLocalIntersection(0.0f);

    // Loop through all triangles using the index buffer
    // (Make sure indices.size() is a multiple of 3)
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        if (i2 >= vertices.size() )
        {
            
            break;   
        }
        // Gather the triangle’s vertices
        const glm::vec3& v0 = vertices[i0];
        const glm::vec3& v1 = vertices[i1];
        const glm::vec3& v2 = vertices[i2];
        
        
        float t, u, v;
        if (MollerTrumboreIntersection(localRayOrigin, localRayDirection, v0, v1, v2, t, u, v))
        {
            if (t < closestT)
            {
                closestT = t;
                bestLocalIntersection = localRayOrigin + localRayDirection * t;
                hit = true;
               
            }
        }
    }

    if (hit)
    {
        // Transform best local intersection to world space
        intersectionPoint = glm::vec3(transform.model * glm::vec4(bestLocalIntersection, 1.0f));
        tMin = closestT; // parametric distance along the *local* ray
        return true;
    }

    return false;
}