#pragma once

#include "secs.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>

// Example Components
struct TestPosition {
    float x = 0.0f;
    float y = 0.0f;

    TestPosition() = default;
    TestPosition(float _x, float _y) : x(_x), y(_y) {}
};

struct TestVelocity {
    float vx = 0.0f;
    float vy = 0.0f;

    TestVelocity() = default;
    TestVelocity(float _vx, float _vy) : vx(_vx), vy(_vy) {
        std::cout << "DEBUG: TestVelocity(float, float) called with " << _vx << ", " << _vy << "\n";
    }
};

struct TestHealth {
    int hp = 100;
    int maxHp = 100;

    TestHealth() = default;
    TestHealth(int _hp, int _maxHp) : hp(_hp), maxHp(_maxHp) {}
};

// Helper function for checking conditions and logging
inline void SECS_TEST_CHECK(bool condition, const std::string& testDesc) {
    if (condition) {
        std::cout << "[ OK ] " << testDesc << "\n";
    } else {
        std::cout << "[FAIL] " << testDesc << "\n";
        throw std::runtime_error("Test failed: " + testDesc);
    }
}

// Function to register components and return their IDs
inline void RegisterComponents(secs::ComponentRegistry& registry, int& posCompType, int& velCompType, int& healthCompType) {
    posCompType = registry.registerType<TestPosition>("Position");
    velCompType = registry.registerType<TestVelocity>("Velocity");
    healthCompType = registry.registerType<TestHealth>("Health");

    SECS_TEST_CHECK(posCompType == registry.getID<TestPosition>(), "Component type ID test: Position");
    SECS_TEST_CHECK(velCompType == registry.getID<TestVelocity>(), "Component type ID test: Velocity");
    SECS_TEST_CHECK(healthCompType == registry.getID<TestHealth>(), "Component type ID test: Health");
}

// Main test function
inline void RunAllSecsTests(secs::World& world) {
    std::cout << "\n=== SECS TEST SUITE BEGIN ===\n";
    SECS_TEST_CHECK(true, "Sanity check");

    // Register Components
    secs::ComponentRegistry registry;
    int posCompType, velCompType, healthCompType;
    RegisterComponents(registry, posCompType, velCompType, healthCompType);

    // 1. Entity Creation and Removal
    std::cout << "\n-- Testing entity creation / removal --\n";
    auto e1 = world.createEntity();
    auto e2 = world.createEntity();
    auto e3 = world.createEntity();

    SECS_TEST_CHECK(world.isAlive(e1), "Entity e1 should be alive");
    SECS_TEST_CHECK(world.isAlive(e2), "Entity e2 should be alive");
    SECS_TEST_CHECK(world.isAlive(e3), "Entity e3 should be alive");

    world.removeEntity(e3);
    SECS_TEST_CHECK(!world.isAlive(e3), "Entity e3 should be removed / not alive");

    // 2. Add Components
    std::cout << "\n-- Testing add/get components --\n";
    world.addComponent(e1, posCompType, TestPosition{5.0f, 10.0f});
    world.addComponent(e1, velCompType, TestVelocity{1.0f, 0.5f});
    world.addComponent(e2, healthCompType, TestHealth{80, 100});

    // Validate Components
    SECS_TEST_CHECK(!world.hasComponent(e1, healthCompType), "e2 does not have TestVelocity component");

    SECS_TEST_CHECK(world.hasComponent(e1, posCompType), "e1 has TestPosition component");
    SECS_TEST_CHECK(world.hasComponent(e1, velCompType), "e1 has TestVelocity component");

    auto& pos = world.getComponent<TestPosition>(e1, posCompType);
    auto& vel = world.getComponent<TestVelocity>(e1, velCompType);
    SECS_TEST_CHECK(pos.x == 5.0f && pos.y == 10.0f, "TestPosition on e1 is (5,10)");
    SECS_TEST_CHECK(vel.vx == 1.0f && vel.vy == 0.5f, "TestVelocity on e1 is (1, 0.5)");

    SECS_TEST_CHECK(world.hasComponent(e2, healthCompType), "e2 has TestHealth component");
    auto& hp = world.getComponent<TestHealth>(e2, healthCompType);
    SECS_TEST_CHECK(hp.hp == 80 && hp.maxHp == 100, "TestHealth on e2 is hp=80, maxHp=100");

    // Confirm removed entity cannot have components
    try {
        world.addComponent(e3, healthCompType, TestHealth{2, 10});
        SECS_TEST_CHECK(false, "Should not be able to add a component to a removed entity e3!");
    } catch (const std::runtime_error&) {
        SECS_TEST_CHECK(true, "Correctly failed to add a component to a removed entity");
    }

    // 3. Systems and Progress
    std::cout << "\n-- Testing system creation and progress calls --\n";

    secs::System movementSystem({posCompType, velCompType},
        [posCompType, velCompType](secs::Entity entity, secs::World& w) {
            auto& pos = w.getComponent<TestPosition>(entity, posCompType);
            auto& vel = w.getComponent<TestVelocity>(entity, velCompType);
            pos.x += vel.vx;
            pos.y += vel.vy;
            std::cout << "MovementSystem updated entity " << entity << " to TestPosition (" << pos.x << ", " << pos.y << ")\n";
        });

    secs::System healthLoggerSystem({healthCompType},
        [healthCompType](secs::Entity entity, secs::World& w) {
            auto& hp = w.getComponent<TestHealth>(entity, healthCompType);
            std::cout << "HealthLogger sees entity " << entity << " with hp=" << hp.hp << " / " << hp.maxHp << "\n";
        });
    std::vector<secs::System> systems = {movementSystem, healthLoggerSystem};


    // Run systems
    world.progress(&systems);

    auto& updatedPos = world.getComponent<TestPosition>(e1, posCompType);
    SECS_TEST_CHECK(updatedPos.x == 6.0f && updatedPos.y == 10.5f,
                    "movementSystem updated e1's TestPosition to (6, 10.5)");

    world.progress(&systems);
    SECS_TEST_CHECK(updatedPos.x == 7.0f && updatedPos.y == 11.0f,
                    "movementSystem updated e1's TestPosition again to (7, 11)");

    // 4. Removing Entities with Active Components
    std::cout << "\n-- Testing removing entity with active components --\n";
    world.removeEntity(e1);
    SECS_TEST_CHECK(!world.isAlive(e1), "Entity e1 is now removed");

    std::cout << "Running systems after removing e1...\n";
    world.progress(&systems);

    try {
        world.getComponent<TestPosition>(e1, posCompType);
        SECS_TEST_CHECK(false, "Should not be able to get components from removed entity e1!");
    } catch (const std::runtime_error&) {
        SECS_TEST_CHECK(true, "Correctly failed to retrieve component from removed e1");
    }

    std::cout << "\n=== SECS TEST SUITE COMPLETE ===\n";
}
