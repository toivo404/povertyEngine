#pragma once
#include "PEPhysics.h"
// PEPhysicsTests Class Declaration
class PEPhysicsTests {
public:
    // Run all the tests
    static void RunAllTests();

private:
    // Individual tests
    static void TestEmptyMesh();
    static void TestSinglePointMesh();
    static void TestMultiplePointsMesh();
    static void TestRaycastAABB();
};

