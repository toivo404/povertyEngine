#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <any>
#include <vector>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <typeindex>
#include <unordered_set>

// Simple definitions
namespace secs {

using Entity = uint32_t;

// Keep a global or per-world counter for unique component IDs
    class ComponentRegistry {
    public:
        // Registers a type and returns its unique ID
        template <typename T>
        int registerType(const std::string& name) {
            auto typeIdx = std::type_index(typeid(T));
            if (typeToID.find(typeIdx) != typeToID.end()) {
                throw std::runtime_error("Type already registered: " + name);
            }
            int id = nextID++;
            typeToID[typeIdx] = id;
            idToName[id] = name;
            return id;
        }

        // Get the ID for a registered type
        template <typename T>
        int getID() const {
            auto typeIdx = std::type_index(typeid(T));
            auto it = typeToID.find(typeIdx);
            if (it == typeToID.end()) {
                throw std::runtime_error("Type not registered!");
            }
            return it->second;
        }

        // Get the name for a registered ID
        std::string getName(int id) const {
            auto it = idToName.find(id);
            if (it == idToName.end()) {
                throw std::runtime_error("Invalid ID!");
            }
            return it->second;
        }

    private:
        int nextID = 0;
        std::unordered_map<std::type_index, int> typeToID;
        std::unordered_map<int, std::string> idToName;
    };


// A basic world that holds entities & their components
class World {
public:
    // Creates a new entity (just increments an ID)
    Entity createEntity() {
        Entity e = ++m_nextEntity;
        m_entityComponents[e] = {}; // Initialize the entity in the map
        return e;
    }

    // Removes an entity by marking it as removed and clearing its components
    void removeEntity(Entity e) {
        auto it = m_entityComponents.find(e);
        if (it != m_entityComponents.end()) {
            m_removedEntities.insert(e); // Mark as removed
            m_entityComponents.erase(it); // Remove its components
        }
    }

    bool isAlive(Entity e) const {
        return (m_entityComponents.find(e) != m_entityComponents.end() && 
                m_removedEntities.find(e) == m_removedEntities.end());
    }

    // Adds (or replaces) a component for an entity
    void addComponent(Entity e, int componentID, std::any data) {
        if (!isAlive(e)) {
            throw std::runtime_error("Cannot add component to a removed or nonexistent entity!");
        }
        m_entityComponents[e][componentID] = std::move(data);
    }

    // Check if an entity has a component
    bool hasComponent(Entity e, int componentID) const {
        auto entIt = m_entityComponents.find(e);
        if (entIt == m_entityComponents.end()) return false;
        return entIt->second.find(componentID) != entIt->second.end();
    }

    // Retrieve the std::any for a given component
    std::any& getComponentAny(Entity e, int componentID) {
        auto entIt = m_entityComponents.find(e);
        if (entIt == m_entityComponents.end() || m_removedEntities.count(e)) {
            throw std::runtime_error("Entity does not exist or is removed");
        }
        auto compIt = entIt->second.find(componentID);
        if (compIt == entIt->second.end()) {
            throw std::runtime_error("Entity does not have componentID: " + std::to_string(componentID));
        }
        return compIt->second;
    }

    // Helper: cast the stored std::any to a T&
    template <typename T>
    T& getComponent(Entity e, int componentID) {
        return std::any_cast<T&>(getComponentAny(e, componentID));
    }

    // For iteration across all entities
    std::vector<Entity> getAllEntities() const {
        std::vector<Entity> result;
        for (const auto& kv : m_entityComponents) {
            if (m_removedEntities.find(kv.first) == m_removedEntities.end()) {
                result.push_back(kv.first);
            }
        }
        return result;
    }

private:
    Entity m_nextEntity = 0;
    std::unordered_map<Entity, std::unordered_map<int, std::any>> m_entityComponents;
    std::unordered_set<Entity> m_removedEntities; // Track removed entities
};

// A System that has a set of required componentIDs, plus a callback
// that processes those components for each entity that has them.
class System {
public:
    using SystemCallback = std::function<void(Entity, World&)>;

    // Pass in a list of component IDs that this system requires
    System(std::vector<int> requiredComponents, SystemCallback cb)
        : m_required(std::move(requiredComponents)), m_callback(std::move(cb))
    {}

    void execute(World &world) {
        // Very naive iteration: check every entity
        auto entities = world.getAllEntities();
        for (auto e : entities) {
            if (!world.isAlive(e)) {
                continue;
            }
            // Check if entity has all required comps
            bool hasAll = true;
            for (int compID : m_required) {
                if (!world.hasComponent(e, compID)) {
                    hasAll = false;
                    break;
                }
            }
            if (hasAll) {
                // Run the system callback
                m_callback(e, world);
            }
        }
    }

private:
    std::vector<int> m_required; 
    SystemCallback m_callback;
};

// Helper to run multiple systems
inline void progress(World& world, const std::vector<System*>& systems) {
    for (auto* sys : systems) {
        sys->execute(world);
    }
}

} // namespace iecs
