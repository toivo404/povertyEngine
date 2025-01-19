#include "PEPhysicsTests.h"

#include <vector>
#include <glm.hpp>
#include <iostream>
#include <ostream>
#include <PEPhysics.h>
#include "systems/TransformSystem.h"

void PEPhysicsTests::TestEmptyMesh() {
    try {
        PEPhysics::CalculateBoundingBox({});
        std::cerr << "TestEmptyMesh failed: No exception thrown for empty mesh.\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "TestEmptyMesh passed.\n";
    }
}

void PEPhysicsTests::TestSinglePointMesh() {
    std::vector<glm::vec3> vertices = {{1.0f, 2.0f, 3.0f}};
    auto aabb = PEPhysics::CalculateBoundingBox(vertices);

    if (aabb.min == vertices[0] && aabb.max == vertices[0]) {
        std::cout << "TestSinglePointMesh passed.\n";
    } else {
        std::cerr << "TestSinglePointMesh failed.\n";
    }
}

void PEPhysicsTests::TestMultiplePointsMesh() {
    std::vector<glm::vec3> vertices = {
        {1.0f, 2.0f, 3.0f},
        {-1.0f, 4.0f, 0.0f},
        {2.0f, -3.0f, 1.0f},
        {0.0f, 0.0f, -2.0f}
    };
    auto aabb = PEPhysics::CalculateBoundingBox(vertices);
    glm::vec3 expectedMin(-1.0f, -3.0f, -2.0f);
    glm::vec3 expectedMax(2.0f, 4.0f, 3.0f);

    if (aabb.min == expectedMin && aabb.max == expectedMax) {
        std::cout << "TestMultiplePointsMesh passed.\n";
    } else {
        std::cerr << "TestMultiplePointsMesh failed.\n";
    }
}

 void PEPhysicsTests::TestRaycastAABB() {
    AABB aabb = {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    Transform transform;
    transform.position = glm::vec3(3.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    transform.UpdateModelMatrix();
    
    glm::vec3 rayOrigin(0.0f, 0.0f, 0.0f);
    glm::vec3 rayDirection(1.0f, 0.0f, 0.0f);

    float tMin;
    glm::vec3 intersectionPoint;
    bool hit = PEPhysics::RaycastAABB(rayOrigin, rayDirection, aabb, transform, tMin, intersectionPoint);

    float distance = glm::distance(intersectionPoint, glm::vec3(1.0f, 0.0f, 0.0f));
    if (hit && distance < 0.001f) {
        std::cout << "TestRaycastAABB passed.\n";
    } else {
        std::cerr << "TestRaycastAABB failed. hit: " << std::to_string(hit) << "dist" << std::to_string(distance) << std::endl;
    }
    
}

void PEPhysicsTests::RunAllTests()
{
    std::cout << "==== PEPhysics tests ====" << std::endl;
    TestEmptyMesh();
    TestSinglePointMesh();
    TestMultiplePointsMesh();
    TestRaycastAABB();
    std::cout << "==========================" << std::endl;
}
