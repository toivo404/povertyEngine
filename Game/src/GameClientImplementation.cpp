#include "GameClientImplementation.h"
#include "systems/TransformSystem.h"

struct Spin {
    float rotateSpeed = 50.0f; // Rotation speed in degrees per second
};
struct HelloWorldComponent
{
    int number = 42;
};
struct Manipulator {
    float moveSpeed   = 2.0f;  
    float rotateSpeed = 50.0f;
};


int add(int a, int b) {
    return a + b;
}
std::vector<secs::Entity> placedAssets;

secs::Entity GameClientImplementation::PlaceAsset(

    const glm::vec3& position,
    const std::string& materialFolder,
    const std::string& modelPath)
{
    // Register component types (if not already registered)
    static int transformTypeId = componentRegistry.getID<Transform>();
    static int shaderTypeId = componentRegistry.getID<Shader>();
    static int materialTypeId = componentRegistry.getID<Material>();
    static int meshTypeId = componentRegistry.getID<Mesh>();
 
    // Create a new entity
    secs::Entity entity = world.createEntity();

    // Add components to the entity
    world.addComponent(entity, transformTypeId, Transform{position, glm::vec3(1.0f), glm::vec3(0.0f)});
    const auto& transform = world.getComponent<Transform>(entity, transformTypeId);
    std::cout << "Transform added: position = (" << transform.position.x << ", " 
              << transform.position.y << ", " << transform.position.z << ")\n";
    world.addComponent(entity, shaderTypeId, Shader{Engine::GetShader("basic")});
    world.addComponent(entity, materialTypeId, Material{Engine::GetMaterial(materialFolder)});
    world.addComponent(entity, meshTypeId, Mesh{Engine::GetMesh(modelPath)});
    world.addComponent(entity, componentRegistry.getID<Spin>(), Spin{50});
    

    return entity;
}

glm::vec3 GetRandomColor()
{
    const glm::vec3 baseColor(0.6f, 0.2f, 0.8f);
    return  baseColor + glm::vec3(
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f,
         static_cast<float>(rand()) / RAND_MAX * 0.2f - 0.1f
     );
}

void GameUpdate()
{
    std::cout << "hello hello" << std::endl;
   
}
/*
void PopulateMonkeys(secs::World& world, secs::ComponentRegistry& registry) {
    const float spacing = 2.5f; 
    const int count = 20;      
    for (int i = 0; i < count; ++i) {
        float x = (i % 5) * spacing - 5.0f;
        float z = (i / 5) * spacing - 5.0f;
        GameClientImplementation::PlaceAsset(glm::vec3(x, 0.0f, z), "assets/models/monkey/", "assets/models/monkey/monkey.fbx");
    }
}
*/


void GameClientImplementation::OnInit()
{
    RegisterClientComponents();
    RegisterClientSystems();

    int transformTypeId = componentRegistry.getID<Transform>();
    int cameraTypeId = componentRegistry.getID<Camera>();
    int manipulatorTypeId = componentRegistry.getID<Manipulator>();
    int lightTypeId = componentRegistry.getID<Light>();
    int spinTypeId = componentRegistry.getID<Spin>();
    
    // Calculate initial values for camera
    glm::vec3 origCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 origDirection = glm::normalize(origCamTarget - Engine::camPos);
    float pitch = glm::degrees(asin(origDirection.y));
    float yaw = glm::degrees(atan2(origDirection.z, origDirection.x));

    // Create and set up "Main Camera" entity
    secs::Entity mainCamera = world.createEntity();
    world.addComponent(mainCamera, transformTypeId, Transform{
        glm::vec3(0.0f, 5.0f, 10.0f), // Position
        glm::vec3(1.0f),              // Scale
        glm::vec3(pitch, yaw, 0.0f)   // Rotation
    });
    world.addComponent(mainCamera, cameraTypeId, Camera{42});
    world.addComponent(mainCamera, manipulatorTypeId, Manipulator{2, 50});

    // Create and set up "Main Light" entity
    secs::Entity mainLight = world.createEntity();
    world.addComponent(mainLight, transformTypeId, Transform{
        glm::vec3(0.0f, 0.0f, 0.0f), // Position
        glm::vec3(0, 0, 0),          // Scale
        glm::vec3(15, 0, 0.0f)       // Rotation
    });
    world.addComponent(mainLight, lightTypeId, Light{
        glm::vec3(1.0f, 1.0f, 1.0f) // Color
    });
    world.addComponent(mainLight, spinTypeId, Spin{50});
        

}

void GameClientImplementation::RegisterClientComponents()
{
    componentRegistry.registerType<Spin>("Spin");
    componentRegistry.registerType<Manipulator>("Manipulator");
    componentRegistry.registerType<HelloWorldComponent>("HelloWorldComponent");

}


void GameClientImplementation::RegisterClientSystems()
{

    int spinTypeId = componentRegistry.getID<Spin>();
    int transformTypeId = componentRegistry.getID<Transform>();
    int manipulatorTypeId = componentRegistry.getID<Manipulator>();

    secs::System spinTransformSystem({spinTypeId, transformTypeId},
            [spinTypeId, transformTypeId](secs::Entity e, secs::World& w) {
                auto& spin = w.getComponent<Spin>(e, spinTypeId);
                auto& transform = w.getComponent<Transform>(e, transformTypeId);

                transform.rotation.y += spin.rotateSpeed * Engine::deltaTime;
                transform.position.y += Engine::deltaTime;
                transform.rotation.y = glm::mod(transform.rotation.y, 360.0f);
            });
    Engine::AddSystem(spinTransformSystem);


    secs::System manipulatorTransformSystem({manipulatorTypeId, transformTypeId},
        [manipulatorTypeId, transformTypeId](secs::Entity e, secs::World& w) {
            auto& manipulator = w.getComponent<Manipulator>(e, manipulatorTypeId);
            auto& transform = w.getComponent<Transform>(e, transformTypeId);

            if (Engine::IsKeyPressed(SDLK_w)) { transform.position.z -= manipulator.moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_s)) { transform.position.z += manipulator.moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_a)) { transform.position.x -= manipulator.moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_d)) { transform.position.x += manipulator.moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_z)) { transform.position.y -= manipulator.moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_x)) { transform.position.y += manipulator.moveSpeed * Engine::deltaTime; }

            if (Engine::IsKeyPressed(SDLK_UP))    { transform.rotation.x -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_DOWN))  { transform.rotation.x += manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_LEFT))  { transform.rotation.y -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_RIGHT)) { transform.rotation.y += manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_q))     { transform.rotation.z -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_e))     { transform.rotation.z += manipulator.rotateSpeed * Engine::deltaTime; }
        });
    Engine::AddSystem(manipulatorTransformSystem);

    int helloWorldCompId = componentRegistry.getID<HelloWorldComponent>();
    secs::System helloWorldSystem({helloWorldCompId, transformTypeId},
                                  [helloWorldCompId, transformTypeId](secs::Entity e, secs::World& w) {
                                      std::cout << "hello world" << std::endl;
                                  });
    Engine::AddSystem(helloWorldSystem);

}

void GameClientImplementation::OnUpdate(float deltaTime)
{
    if (Engine::GetKeyUp(SDLK_PERIOD))
    {
        auto asset = PlaceAsset( glm::vec3(placedAssets.size(), 0, 0), "assets/models/monkey/", "assets/models/monkey/monkey.fbx");
        placedAssets.push_back(asset);
        std::cout << "Assets: " << placedAssets.size() << std::endl;
    }
    if (Engine::GetKeyUp(SDLK_COMMA))
    {
        if (!placedAssets.empty())
        {
            auto lastMonkey = placedAssets.back();
            world.removeEntity(lastMonkey);
            placedAssets.pop_back();
            std::cout << "Assets " << placedAssets.size() << std::endl;
        }
        else
        {
            std::cout << "Scene empty" << std::endl;
        }
    }
}

void GameClientImplementation::OnShutdown()
{
}
