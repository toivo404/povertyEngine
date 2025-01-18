#include <iostream>
#include "secs.h"
#include <stdexcept>
#define SECS_DEBUG

namespace secs {


// World Implementation
Entity World::createEntity() {
    Entity e = ++m_nextEntity;
    m_entityComponents[e] = {}; // Initialize the entity in the map
    return e;
}

void World::removeEntity(Entity e) {
    auto it = m_entityComponents.find(e);
    if (it != m_entityComponents.end()) {
        m_removedEntities.insert(e); // Mark as removed
        m_entityComponents.erase(it); // Remove its components
    }
}

bool World::isAlive(Entity e) const {
    return (m_entityComponents.find(e) != m_entityComponents.end() &&
            m_removedEntities.find(e) == m_removedEntities.end());
}

void World::addComponent(Entity e, int componentID, std::any data) {
    if (!isAlive(e)) {
        throw std::runtime_error("Cannot add component to a removed or nonexistent entity!");
    }
    m_entityComponents[e][componentID] = std::move(data);
}

bool World::hasComponent(Entity e, int componentID) const {
    auto entIt = m_entityComponents.find(e);
    if (entIt == m_entityComponents.end()) return false;
    return entIt->second.find(componentID) != entIt->second.end();
}
    

std::vector<Entity> World::getAllEntities() const {
    std::vector<Entity> result;
    for (const auto& kv : m_entityComponents) {
        if (m_removedEntities.find(kv.first) == m_removedEntities.end()) {
            result.push_back(kv.first);
        }
    }
    return result;
}

void World::progress(std::vector<secs::System>* systems)
{
    if (!systems) {
        throw std::runtime_error("Systems pointer is null");
    }
    for (auto& sys : *systems) { // Dereference the pointer
        sys.execute(*this);
    }
}

/*// System Implementation
    System::System(std::vector<int> requiredComponents, SystemCallback cb)
        : m_required(std::move(requiredComponents)), m_callback(std::move(cb)) {}

void System::execute(World& world) {
    auto entities = world.getAllEntities();
    for (auto e : entities) {
        if (!world.isAlive(e)) {
            continue;
        }
        bool hasAll = true;
        for (int compID : m_required) {
            if (!world.hasComponent(e, compID)) {
                hasAll = false;
                break;
            }
        }
        if (hasAll) {
            m_callback(e, world);
        }
    }
}
*/
} // namespace secs
