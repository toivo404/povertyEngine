#pragma once

#include <core.h> 
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <any>
#include <vector>
#include <functional>
#include <stdexcept>
#include <typeindex>

namespace secs {

    using Entity = uint32_t;

    class System; // Forward declaration
    struct Name
    {
        std::string name;
    };
    // Component Registry
   
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

    // World
    class PE_API World {
    public:
        Entity createEntity();
        void removeEntity(Entity e);
        bool isAlive(Entity e) const;

        void addComponent(Entity e, int componentID, std::any data);
        bool hasComponent(Entity e, int componentID) const;

        // Retrieve the std::any for a given component
        std::any* getComponentAny(Entity e, int componentID) {
            auto entIt = m_entityComponents.find(e);
            if (entIt == m_entityComponents.end() || m_removedEntities.count(e)) {
                return nullptr; // Entity does not exist or is removed
            }
            auto compIt = entIt->second.find(componentID);
            if (compIt == entIt->second.end()) {
                return nullptr; // Component does not exist
            }
            return &compIt->second; // Return pointer to the component
        }

        template <typename T>
        T* tryGetComponent(Entity e, int componentID)
        {
            std::any* compAny = getComponentAny(e, componentID);
            if (compAny)
            {
                try
                {
                    return &std::any_cast<T&>(*compAny); // Return pointer to the stored component
                }
                catch (const std::bad_any_cast&)
                {
                    throw std::runtime_error("Type mismatch in tryGetComponent: requested type does not match stored type.");
                }
            }
            return nullptr; // Component does not exist
        }

        template <typename T>
        T& getComponent(Entity e, int componentID) {
            const auto comp = getComponentAny(e, componentID);
            if (comp == nullptr)
                throw std::runtime_error("Cannot find component with ID " + std::to_string(componentID) + " on entity " + std::to_string(e));
            return std::any_cast<T&>(*comp);
        }

        std::vector<Entity> getAllEntities() const;

        void progress(std::vector<System>* systems);

    private:
        Entity m_nextEntity = 0;
        std::unordered_map<Entity, std::unordered_map<int, std::any>> m_entityComponents;
        std::unordered_set<Entity> m_removedEntities; // Track removed entities
    };

    // System
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

} // namespace secs
