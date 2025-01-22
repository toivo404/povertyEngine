#pragma once
#include <iostream>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <typeindex>
#include <algorithm>
#include <memory>
#include <cassert>
#include <functional>
#include <cstring>
/*
    =================================================
    SECS: SUPER ENTITY COMPONENT SYSTEM
    =================================================
*/
namespace secs
{

/*
    ===================
    COMPONENT REGISTRY
    ===================
    - Keeps a mapping from C++ type to a unique integer ID.
    - Stores the size of each component type for chunk/SoA allocations.
    - ECS code uses only integer IDs to identify components.
*/
class ComponentRegistry {
public:
    // Register a new component type T with an internal integer ID.
    template<typename T>
    static int registerType(const std::string& name) {
        std::type_index tIndex(typeid(T));
        // If already registered, just return the existing ID
        if (typeToID.find(tIndex) != typeToID.end()) {
            return typeToID[tIndex];
        }
        // Otherwise, assign a new ID
        int newID = nextID++;
        std::cout << "Registered type:: " << newID << " name: " << name << std::endl;
        typeToID[tIndex] = newID;
        idToSize[newID]   = sizeof(T);
        idToName[newID]   = name;
        return newID;
    }

    // Retrieve the unique ID for a component T
    template<typename T>
    static int getID() {
        std::type_index tIndex(typeid(T));
        auto it = typeToID.find(tIndex);
        if (it == typeToID.end()) {
            throw std::runtime_error("Type not registered with ComponentRegistry!");
        }
        return it->second;
    }

    // Get the size of a component by its ID
    static size_t getSize(int compID) {
        auto it = idToSize.find(compID);
        if (it == idToSize.end()) {
            throw std::runtime_error("getSize called on unknown component ID!");
        }
        return it->second;
    }

    // (Optional) name retrieval for debugging
    static const std::string& getName(int compID) {
        auto it = idToName.find(compID);
        if (it == idToName.end()) {
            static std::string unknown = "UnknownComponent";
            return unknown;
        }
        return it->second;
    }

private:
    // Next available integer for a new component type
    static inline int nextID = 0;

    // Maps from a std::type_index -> assigned ID
    static inline std::unordered_map<std::type_index, int> typeToID;
    // Maps from ID -> size
    static inline std::unordered_map<int, size_t> idToSize;
    // Maps from ID -> a friendly name (for debugging)
    static inline std::unordered_map<int, std::string> idToName;
};

/*
    ===================
    ENTITY
    ===================
    Just an ID. Add a version if you want.
*/
struct Entity {
    uint32_t id;
};

/*
    ===================
    SIGNATURE BUILDING
    ===================
    We create a canonical string from a list of component IDs
    so we can identify & cache Archetypes in a map.
*/
inline std::string buildSignature(const std::vector<int>& compIDs)
{
    // Sort them for stable ordering
    std::vector<int> sorted = compIDs;
    std::sort(sorted.begin(), sorted.end());

    // Build a string like "1;5;7;" for an archetype that has compIDs = {1,5,7}
    std::string sig;
    for (auto id : sorted) {
        sig += std::to_string(id) + ";";
    }
    return sig;
}

/*
    ===================
    ARCHETYPE
    ===================
    Groups entities that have the same set of components (same signature).
    Stores data in a SoA layout for each component ID.
*/
class Archetype
{
public:
    // Construct from a set of component IDs
    Archetype(std::vector<int> compIDs)
        : componentIDs(std::move(compIDs))
    {
        // Sort them for consistent ordering
        std::sort(componentIDs.begin(), componentIDs.end());
    }

    // Store definition for the raw data buffer that holds each component's bytes
    struct ComponentData {
        int compID;
        size_t componentSize;
        std::vector<uint8_t> data; // SoA data
    };

    // Initialize storage for a particular component ID (called once at creation time)
    void initializeComponentStorage(int compID, size_t size)
    {
        ComponentData cData{ compID, size, {} };
        compMap[compID] = static_cast<int>(components.size());
        components.push_back(std::move(cData));
    }

    // Check if this archetype has a given component ID
    bool hasComponent(int compID) const {
        return (compMap.find(compID) != compMap.end());
    }

    // Insert a new entity. Returns the index in the SoA buffers
    size_t addEntity(Entity e)
    {
        size_t newIndex = entities.size();
        entities.push_back(e);
        entityToIndex[e.id] = newIndex;

        // Expand each SoA data buffer by componentSize
        for (auto& comp : components) {
            size_t oldSizeBytes = comp.data.size();
            size_t newSizeBytes = oldSizeBytes + comp.componentSize;
            comp.data.resize(newSizeBytes, 0); // zero out new area
        }
        return newIndex;
    }

    // Remove an entity with typical "swap-and-pop" to maintain a tight array
    void removeEntity(Entity e)
    {
        auto it = entityToIndex.find(e.id);
        if (it == entityToIndex.end()) {
            return; // Not in this archetype
        }

        size_t index     = it->second;
        size_t lastIndex = entities.size() - 1;

        if (index != lastIndex)
        {
            // Swap in the last entity
            Entity lastEnt = entities[lastIndex];
            entities[index] = lastEnt;
            entityToIndex[lastEnt.id] = index;

            // Swap the SoA data for each component
            for (auto& comp : components) {
                size_t sizeBytes = comp.componentSize;
                uint8_t* arr     = comp.data.data();
                size_t offsetA   = index * sizeBytes;
                size_t offsetB   = lastIndex * sizeBytes;

                for (size_t i = 0; i < sizeBytes; ++i) {
                    std::swap(arr[offsetA + i], arr[offsetB + i]);
                }
            }
        }

        // Pop the last entity
        entities.pop_back();
        entityToIndex.erase(e.id);

        // Shrink each SoA data buffer
        for (auto& comp : components) {
            comp.data.resize(comp.data.size() - comp.componentSize);
        }
    }

    // Get pointer to the raw bytes of compID for entity e
    // Returns nullptr if e not found or if compID not in this archetype
    uint8_t* getComponentData(Entity e, int compID)
    {
        auto it = entityToIndex.find(e.id);
        if (it == entityToIndex.end()) {
            return nullptr;
        }
        auto compIt = compMap.find(compID);
        if (compIt == compMap.end()) {
            return nullptr;
        }
        size_t entityIndex = it->second;
        int compVecIndex   = compIt->second;
        ComponentData& c   = components[compVecIndex];
        size_t offset      = entityIndex * c.componentSize;
        return c.data.data() + offset;
    }

    // Accessors
    size_t getEntityCount() const { return entities.size(); }
    const std::vector<Entity>& getEntities() const { return entities; }
    const std::vector<int>& getComponentIDs() const { return componentIDs; }

    // Publicly accessible only if needed (e.g., for moving data):
    std::vector<ComponentData> components;
    std::unordered_map<int, int> compMap;

private:
    std::vector<int> componentIDs;
    std::vector<Entity> entities;
    std::unordered_map<uint32_t, size_t> entityToIndex;
};

/*
    ===================
    WORLD
    ===================
    - Manages archetypes for every unique combination of components.
    - Tracks which archetype each entity belongs to.
    - Creates/destroys entities and moves them between archetypes as comps are added/removed.
*/
class World
{
public:
    World() = default;

    // Prevent copying to avoid attempts to copy unique_ptr in archetypes map
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // You may allow move semantics if desired
    World(World&&) = default;
    World& operator=(World&&) = default;

    // Create entity with a set of component IDs
    Entity createEntity(const std::vector<int>& compIDs)
    {
        Entity e{nextEntityID++};
        Archetype* arch = getOrCreateArchetype(compIDs);
        arch->addEntity(e);
        entityArchetypeMap[e.id] = arch;
        return e;
    }

    // Destroy an entity
    void destroyEntity(Entity e)
    {
        auto it = entityArchetypeMap.find(e.id);
        if (it == entityArchetypeMap.end())
        {
            return; // Already destroyed or invalid
        }
        it->second->removeEntity(e);
        entityArchetypeMap.erase(e.id);
    }

    bool isAlive(Entity e) const
    {
        return (entityArchetypeMap.find(e.id) != entityArchetypeMap.end());
    }

    // Add a component T to an entity (runtime)
    template<typename T>
    void addComponent(Entity e, const T& value = T{})
    {
        Archetype* oldArch = findArchetype(e);
        if (!oldArch) return;

        // Build new signature (old + T)
        std::vector<int> newCompIDs = oldArch->getComponentIDs();
        int newID = ComponentRegistry::getID<T>();
        if (std::find(newCompIDs.begin(), newCompIDs.end(), newID) != newCompIDs.end()) {
            // Already has it => Just overwrite
            T* ptr = reinterpret_cast<T*>(oldArch->getComponentData(e, newID));
            if (ptr) *ptr = value;
            return;
        }
        newCompIDs.push_back(newID);

        Archetype* newArch = getOrCreateArchetype(newCompIDs);
        moveEntityToArchetype(e, oldArch, newArch);

        // Set the newly added component data
        T* ptr = reinterpret_cast<T*>(newArch->getComponentData(e, newID));
        if (ptr) {
            *ptr = value;
        }
    }

    // Remove a component T from an entity (runtime)
    template<typename T>
    void removeComponent(Entity e)
    {
        Archetype* oldArch = findArchetype(e);
        if (!oldArch) return;

        std::vector<int> newCompIDs = oldArch->getComponentIDs();
        int remID = ComponentRegistry::getID<T>();
        auto it   = std::find(newCompIDs.begin(), newCompIDs.end(), remID);
        if (it == newCompIDs.end()) {
            // Not present
            return;
        }
        newCompIDs.erase(it);

        Archetype* newArch = getOrCreateArchetype(newCompIDs);
        moveEntityToArchetype(e, oldArch, newArch);
        // Now T is gone, as newArch doesn't contain that compID
    }

    // Retrieve a component T from an entity
    template<typename T>
    T* getComponent(Entity e)
    {
        Archetype* arch = findArchetype(e);
        if (!arch) return nullptr;

        int compID = ComponentRegistry::getID<T>();
        uint8_t* bytes = arch->getComponentData(e, compID);
        return reinterpret_cast<T*>(bytes);
    }

    // Let systems iterate archetypes if needed
    const std::unordered_map<std::string, std::unique_ptr<Archetype>>& getAllArchetypes() const
    {
        return archetypes;
    }

private:
    Archetype* findArchetype(Entity e)
    {
        auto it = entityArchetypeMap.find(e.id);
        if (it == entityArchetypeMap.end()) {
            return nullptr;
        }
        return it->second;
    }

    // Create/fetch archetype for a set of component IDs
    Archetype* getOrCreateArchetype(const std::vector<int>& compIDs)
    {
        std::string sig = buildSignature(compIDs);
        auto it = archetypes.find(sig);
        if (it != archetypes.end()) {
            return it->second.get();
        }

        // New archetype
        auto newArch = std::make_unique<Archetype>(compIDs);
        for (int cID : compIDs) {
            size_t cSize = ComponentRegistry::getSize(cID);
            newArch->initializeComponentStorage(cID, cSize);
        }

        Archetype* ptr = newArch.get();
        archetypes[sig] = std::move(newArch);
        return ptr;
    }

    // Moves an entity between two archetypes, copying shared component data
    void moveEntityToArchetype(Entity e, Archetype* oldArch, Archetype* newArch)
    {
        if (oldArch == newArch) return;

        // 1) Copy data for all overlapping components
        auto& oldIDs = oldArch->getComponentIDs();
        auto& newIDs = newArch->getComponentIDs();

        // We'll store data temporarily here
        std::unordered_map<int, std::vector<uint8_t>> transferData;

        // Collect data from old arch for components that exist in new arch
        for (int cid : oldIDs)
        {
            if (std::find(newIDs.begin(), newIDs.end(), cid) != newIDs.end()) {
                // Overlapping component
                uint8_t* oldBytes = oldArch->getComponentData(e, cid);
                if (oldBytes) {
                    size_t cSize = ComponentRegistry::getSize(cid);
                    transferData[cid].resize(cSize);
                    std::memcpy(transferData[cid].data(), oldBytes, cSize);
                }
            }
        }

        // 2) Remove from old arch
        oldArch->removeEntity(e);

        // 3) Add to new arch
        newArch->addEntity(e);

        // 4) Copy data into new arch
        for (auto& [cid, buf] : transferData)
        {
            uint8_t* newBytes = newArch->getComponentData(e, cid);
            if (newBytes) {
                std::memcpy(newBytes, buf.data(), buf.size());
            }
        }

        // 5) Update the map
        entityArchetypeMap[e.id] = newArch;
    }


    uint32_t nextEntityID = 0;
    std::unordered_map<std::string, std::unique_ptr<Archetype>> archetypes;
    std::unordered_map<uint32_t, Archetype*> entityArchetypeMap;
};

/*
    ===================
    SYSTEM
    ===================
    - Has a list of required component IDs.
    - On execute, it finds all archetypes containing them
      and processes each matching entity with a user callback.
*/
class System
{
public:
    using Callback = std::function<void(Entity, World&)>;

    System(std::vector<int> requiredCompIDs, Callback cb)
        : requiredComponents(std::move(requiredCompIDs))
        , callback(std::move(cb))
    {
        std::sort(requiredComponents.begin(), requiredComponents.end());
    }

    void execute(World& world)
    {
        const auto& allArche = world.getAllArchetypes();
        for (auto& [sig, archPtr] : allArche)
        {
            if (matchesAll(archPtr.get())) {
                const auto& ents = archPtr->getEntities();
                for (auto& e : ents) {
                    callback(e, world);
                }
            }
        }
    }

private:
    bool matchesAll(const Archetype* arch) const
    {
        for (int compID : requiredComponents) {
            if (!arch->hasComponent(compID)) return false;
        }
        return true;
    }

    std::vector<int> requiredComponents;
    Callback callback;
};


// ============ Detail namespace for type expansion trick ============
namespace detail
{
    // We define a helper that, given the user callback, the arrays, and the entity list,
    // calls: callback(entities, typedPtr1, typedPtr2, ..., count)

    template <typename... Components, typename ChunkCallback, size_t... Indices>
    void invokeChunkCallbackImpl(
        ChunkCallback& cb,
        const std::vector<Entity>& entities,
        const std::vector<void*>& arrays,
        size_t entityCount,
        std::index_sequence<Indices...>)
    {
        // We'll cast each arrays[Indices] to the corresponding type pointer.
        // So arrays[0] => Components0 pointer, arrays[1] => Components1 pointer, etc.
        cb(
            entities,
            reinterpret_cast<Components*>(arrays[Indices])...,
            entityCount
        );
    }

    template <typename... Components, typename ChunkCallback>
    void invokeChunkCallback(
        ChunkCallback& cb,
        const std::vector<Entity>& entities,
        const std::vector<void*>& arrays,
        size_t entityCount
    )
    {
        // Generate a compile-time index sequence for the size of the parameter pack
        invokeChunkCallbackImpl<Components...>(
            cb,
            entities,
            arrays,
            entityCount,
            std::index_sequence_for<Components...>{}
        );
    }
} // namespace detail

class EntityBuilder {
public:
    explicit EntityBuilder(World& world)
        : world(world), entityCreated(false) {}

    // Create a new entity
    template <typename... Components>
    EntityBuilder& createEntity() {
        entity = world.createEntity({});
        entityCreated = true;
        return *this;
    }

    // Add a component to the entity
    template <typename T>
    EntityBuilder& set(const T& component) {
        if (!entityCreated) {
            throw std::runtime_error("Entity must be created before adding components!");
        }
        world.addComponent<T>(entity, component);
        return *this;
    }

    // Finalize and return the created entity
    Entity build() {
        if (!entityCreated) {
            throw std::runtime_error("Entity has not been created yet!");
        }
        return entity;
    }

private:
    World& world;
    Entity entity;
    bool entityCreated;
};
    
template<typename... Components, typename ChunkCallback>
void queryChunks(World& world, ChunkCallback callback)
{
    // 1) Gather the component IDs for each type
    std::vector<int> requiredIDs = { ComponentRegistry::getID<Components>()... };

    // 2) Iterate all archetypes in the world
    const auto& archMap = world.getAllArchetypes();
    for (auto& [sig, archePtr] : archMap)
    {
        // Check if archetype has *all* required components
        bool hasAll = true;

        std::cout << "Archetype contains components: ";
        for (int id : archePtr->getComponentIDs()) { // Assuming `getComponentTypes` exists
            std::cout << id << " ";
        }
        std::cout << "\n";
        
        for (int cid : requiredIDs) {
            if (!archePtr->hasComponent(cid)) {
                hasAll = false;
                break;
            }
        }
        if (!hasAll) continue;

        // Archetype matches, so we gather pointers to each component array

        // 3) We'll store pointers to each SoA array in the same order as requiredIDs
        //    E.g. for AABB, Transform => aabbPtr, transformPtr
        //    We do a small helper function or lambda that returns a typed pointer from raw bytes.

        auto gatherArrays = [&](int compID) -> void* {
            // 1) Find the SoA buffer for this component
            auto it = archePtr->compMap.find(compID);
            if (it == archePtr->compMap.end()) {
                return nullptr; // Archetype doesn't have compID
            }

            // 2) Grab the corresponding ComponentData
            Archetype::ComponentData& cData = archePtr->components[it->second];
            // 3) Return the base pointer of cData.data (the entire array)
            return cData.data.empty() ? nullptr : cData.data.data();
        };

        // We'll store each array pointer in a local vector of void* so we can pass them in correct order
        std::vector<void*> arrays;
        arrays.reserve(requiredIDs.size());
        for (int cid : requiredIDs)
        {
            std::cout << "  Contains Component ID " << cid << ": "
                << (archePtr->hasComponent(cid) ? "Yes" : "No") << "\n";
            // The base pointer for this component's SoA data
            void* basePtr = gatherArrays(cid);
            arrays.push_back(basePtr);
        }

        // 4) We have the entire block of entities from the archetype
        auto& entities = archePtr->getEntities();
        size_t entityCount = entities.size();
        if (entityCount == 0) {
            continue; // No data to process
        }
        
        // 5) Now we call the user-provided callback, passing:
        //    - The entity array
        //    - A pointer to each SoA block for each type
        //    The user can iterate them in a single pass or do SSE/AVX for the entire chunk.

        // But we need to cast back to the correct type. We can do that with a helper function 
        // that calls the callback with the right typed pointers in the correct order.

        // We'll define a small helper to "expand" the arrays vector into typed pointers:
        // We'll do a compile-time "index trick" with std::index_sequence
        // for a truly type-safe approach.
        
        detail::invokeChunkCallback<Components...>(callback, entities, arrays, entityCount);
    }
}
} // end namespace secs
