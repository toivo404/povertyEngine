#include "GameClientImplementation.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3_loader.h"
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

struct OutOfBoundsDetector
{
    
};

struct DrawAABB{
    
    
};

struct KeepStuffInSight
{
    
};
int add(int a, int b) {
    return a + b;
}
std::vector<secs::Entity> placedAssets;
int transformCompTypeId;

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
    static int aabbId = componentRegistry.getID<AABB>();
    
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
    world.addComponent(entity, aabbId, AABB{Engine::GetAABB(modelPath)});
    world.addComponent(entity, componentRegistry.getID<OutOfBoundsDetector>(), OutOfBoundsDetector{});
    //world.addComponent(entity, componentRegistry.getID<Spin>(), Spin{50});
   // world.addComponent(entity, componentRegistry.getID<DrawAABB>(), DrawAABB{} );

    return entity;
}

float GetRandomValue()
{
    return  static_cast<float>(rand()) / RAND_MAX;
}

glm::vec3 GetRandomColor()
{
    const glm::vec3 baseColor(0.6f, 0.2f, 0.8f);
    return  baseColor + glm::vec3(
         GetRandomValue() * 0.2f - 0.1f,
         GetRandomValue() * 0.2f - 0.1f,
         GetRandomValue() / RAND_MAX * 0.2f - 0.1f
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

secs::Entity mainCamera;

void GameClientImplementation::OnInit()
{
    RegisterClientComponents();
    RegisterClientSystems();

    transformCompTypeId = componentRegistry.getID<Transform>();
     
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
    mainCamera = world.createEntity();
    world.addComponent(mainCamera, transformCompTypeId, Transform{
                           glm::vec3(0.0f, 5.0f, 10.0f), // Position
                           glm::vec3(1.0f),              // Scale
                           glm::vec3(pitch, yaw, 0.0f)   // Rotation
                       });
    world.addComponent(mainCamera, cameraTypeId, Camera{42});
    int keepStuffInSight = componentRegistry.getID<KeepStuffInSight>();
    world.addComponent(mainCamera, manipulatorTypeId, Manipulator{2, 50});
    //  world.addComponent(mainCamera, spinTypeId, Spin{1});
    world.addComponent(mainCamera, keepStuffInSight, KeepStuffInSight{} );
    
    // Create and set up "Main Light" entity
    secs::Entity mainLight = world.createEntity();
    world.addComponent(mainLight, transformCompTypeId, Transform{
        glm::vec3(0.0f, 0.0f, 0.0f), // Position
        glm::vec3(0, 0, 0),          // Scale
        glm::vec3(15, 0, 0.0f)       // Rotation
    });
    world.addComponent(mainLight, lightTypeId, Light{
        glm::vec3(1.0f, 1.0f, 1.0f) // Color
    });


    
    world.addComponent(mainLight, spinTypeId, Spin{50});
        

}

int obDetectorId;

void GameClientImplementation::RegisterClientComponents()
{
    componentRegistry.registerType<Spin>("Spin");
    componentRegistry.registerType<Manipulator>("Manipulator");
    componentRegistry.registerType<HelloWorldComponent>("HelloWorldComponent");
    obDetectorId = componentRegistry.registerType<OutOfBoundsDetector>("OutOfBoundsDetector");
    componentRegistry.registerType<DrawAABB>("DrawAABB");
    componentRegistry.registerType<KeepStuffInSight>("KeepStuffInSight");
}

bool isGameOver = true;

void GameClientImplementation::RegisterClientSystems()
{

    int spinTypeId = componentRegistry.getID<Spin>();
    int transformTypeId = componentRegistry.getID<Transform>();
    int manipulatorTypeId = componentRegistry.getID<Manipulator>();
    

    secs::System spinTransformSystem({spinTypeId, transformTypeId},
            [spinTypeId, transformTypeId](secs::Entity e, secs::World& w) {
                auto& spin = w.getComponent<Spin>(e, spinTypeId);
                auto& transform = w.getComponent<Transform>(e, transformTypeId);
                transform.RotateAroundAxis(glm::vec3(0,1,0), spin.rotateSpeed * Engine::deltaTime);
                transform.position.y += Engine::deltaTime;
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

            if (Engine::IsKeyPressed(SDLK_UP))    { transform.not_rotation.x -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_DOWN))  { transform.not_rotation.x += manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_LEFT))  { transform.not_rotation.y -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_RIGHT)) { transform.not_rotation.y += manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_q))     { transform.not_rotation.z -= manipulator.rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_e))     { transform.not_rotation.z += manipulator.rotateSpeed * Engine::deltaTime; }
        });
    Engine::AddSystem(manipulatorTransformSystem);
    int drawAABBId = componentRegistry.getID<DrawAABB>();
    int aaBBCompTypeID = componentRegistry.getID<AABB>();
    secs::System drawAABBSystem ( {aaBBCompTypeID, transformTypeId, drawAABBId }, [aaBBCompTypeID, transformTypeId]
        (secs::Entity e, secs::World w)
    {
        w.getComponent<AABB>(e, aaBBCompTypeID).DebugDraw(glm::vec3(1,0,0), w.getComponent<Transform>(e, transformTypeId) );
    });
    
    Engine::AddSystem(drawAABBSystem);
                
  // int clickDetector = componentRegistry.getID<ClickDeletor>();
  // int aabbId = componentRegistry.getID<AABB>();
  //// secs::System mouseClick({clickDetector},
  ////                         [clickDetector, transformTypeId, aabbId](secs::Entity e, secs::World w)
  ////                         {
  ////                           
  ////                         });
  // Engine::AddSystem(mouseClick);

    int helloWorldCompId = componentRegistry.getID<HelloWorldComponent>();
    secs::System helloWorldSystem({helloWorldCompId, transformTypeId},
                                  [helloWorldCompId, transformTypeId](secs::Entity e, secs::World& w) {
                                      std::cout << "hello world" << std::endl;
                                  });
    Engine::AddSystem(helloWorldSystem);

    int keepCompId = componentRegistry.getID<KeepStuffInSight>();
    secs::System keepSightSystem
    ({keepCompId, transformTypeId},
     [keepCompId, transformTypeId, aaBBCompTypeID](secs::Entity e, secs::World& w)
     {
         auto& cameraTransform = w.getComponent<Transform>(e, transformTypeId);

         bool first = true;
         AABB worldAABB;
         for (secs::Entity e2 : w.getAllEntities())
         {
             auto entityAABB = w.tryGetComponent<AABB>(e2, aaBBCompTypeID);
             auto entityTrans = w.tryGetComponent<Transform>(e2, transformTypeId);
             if (entityAABB == nullptr || entityTrans == nullptr)
             {
                 continue;
             }

             if (first)
             {
                 worldAABB = {entityTrans->Apply(entityAABB->min), entityTrans->Apply(entityAABB->max)};
                 first = false;
             }
             else
             {
                 worldAABB.Encapsulate(entityTrans->Apply(entityAABB->min));
                 worldAABB.Encapsulate(entityTrans->Apply(entityAABB->max));
             }
         }

         // DEBUG REMOVE

         if (first)
         {
             // No valid AABB found
             return;
         }

         glm::vec3 pos;
         glm::vec3 target;
         worldAABB.AABBView(pos, target, glm::vec3(1.5, 4.5, 1), 5.5f);


       //  Engine::DebugDrawLine(pos, target, glm::vec3(0, 0, 1), 0.1f);
         // Update camera transform
         Engine::camLook = target;
         Engine::camPos = pos;

         //cameraTransform.UpdateModelMatrix();
     });
    Engine::AddSystem(keepSightSystem);
}



double lastMonkeySpawn;
int killedMonkeys;

void GameClientImplementation::DrawCross()
{
    Engine::DebugDrawLine(glm::vec3(0.0,1,0), glm::vec3(0.0,-1,0), glm::vec3(0.0,0,0), 0.1f);
    Engine::DebugDrawLine(glm::vec3(-1,0,0), glm::vec3(1,0,0), glm::vec3(0.0,0,0), 0.1f);
    Engine::DebugDrawLine(glm::vec3(0,0,1), glm::vec3(0,0,-1), glm::vec3(0.0,0,0), 0.1f);
}



/*
void GameClientImplementation::MoveCameraToViewAll()
{
    auto cameraTransform = world.getComponent<Transform>(mainCamera,transformCompTypeId);
    AABB aabb;
    CalculateWorldAABB(aabb);
    glm::vec3 pos;
    glm::vec3 target;
    aabb.AABBView(pos, target);
    cameraTransform.position = pos;
    cameraTransform.LookAt(target);
    cameraTransform.UpdateModelMatrix();
}
*/
void RemoveLastPlaced(secs::World& w)
{
    auto lastMonkey = placedAssets.back();
    w.removeEntity(lastMonkey);
    placedAssets.pop_back();
}
void GameClientImplementation::OnUpdate(float deltaTime)
{
    DrawCross();


    if (Engine::GetKey(SDLK_PERIOD))
    {
        const int amount = std::max(static_cast<int>(placedAssets.size()), 1 );
        const size_t placedCount = amount;
        const auto square = static_cast<size_t>(sqrt(placedCount));
        
        for (int i = 0; i < square; ++i)
        {
            const auto asset = PlaceAsset(glm::vec3(0), "assets/models/monkey/", "assets/models/monkey/monkey.fbx");
            placedAssets.push_back(asset);
        }
        int size = placedAssets.size();
        for (int i = 0; i < size; ++i)
        {
            Transform& t = world.getComponent<Transform>(placedAssets[i], transformCompTypeId);
            size_t row = i / square; // Determine the row
            size_t col = i % square; // Determine the column
            auto pos = glm::vec3(col * 3.0f, 0.0f, row * 3.0f); // Scale to adjust spacing
            // Use spawnPos for placing the asset
            t.position = pos;
        }
        
        std::cout << "Assets: " << placedAssets.size() * 2 << std::endl;
    }
    if (Engine::GetKey(SDLK_COMMA))
    {
        if (!placedAssets.empty())
        {
            if (placedAssets.size() == 1)
            {
                RemoveLastPlaced(world);
            }
            else
            {
                const int amount = static_cast<int>(placedAssets.size()) / 2;
                for (int i = 0; i < amount ; ++i)
                {
                    RemoveLastPlaced(world);
                }
            }
            
            
            std::cout << "Assets " << placedAssets.size() << std::endl;
        }
        else
        {
            std::cout << "Scene empty" << std::endl;
        }
    }
    if(Engine::GetKeyUp(SDLK_BACKSPACE))
    {
        lastMonkeySpawn = -9999;
        isGameOver = false;
    }
    
    if(Engine::GetMouseButtonUp(1))
    {
        std::cout<<"Click" << std::endl;
        glm::vec3 origin;
        glm::vec3 dir;
        PEPhysicsHitInfo hitInfo;
        int aaBBCompTypeID = componentRegistry.getID<AABB>();
        Engine::MousePositionToRay(origin, dir);
        if (PEPhysics::Raycast(origin, dir, world, transformCompTypeId, aaBBCompTypeID, hitInfo))
        {
            world.removeEntity(hitInfo.entity);
            killedMonkeys++;
        }    
    }

    if(!isGameOver)
    {
        for ( secs::Entity e : world.getAllEntities())
        {
            auto trans = world.tryGetComponent<Transform>(e, transformCompTypeId);
            auto obDetector = world.tryGetComponent<OutOfBoundsDetector>(e, obDetectorId);
            if (obDetector != nullptr && trans != nullptr && !Engine::IsOnScreen(trans->position))
            {
                isGameOver = true;
                Engine::DisplayMessage( "You lost the game" );
                world = secs::World{};
            }
        }
        if (Engine::time - lastMonkeySpawn > 3)
        {
            PlaceAsset(glm::vec3(-2 + (GetRandomValue() * 4), 0, 0), "assets/models/monkey/", "assets/models/monkey/monkey.fbx");
            lastMonkeySpawn = Engine::time;
        }
    
    }
}

void GameClientImplementation::OnShutdown()
{
}
