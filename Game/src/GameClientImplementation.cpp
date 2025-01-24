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


int add(int a, int b) {
    return a + b;
}
std::vector<secs::Entity> placedAssets;
int transformCompTypeId;

secs::Entity GameClientImplementation::PlaceAsset(

    const glm::vec3& position,
    const std::string& materialFile,
    const std::string& modelPath)
{
    // Create a new entity
    return secs::EntityBuilder(world)
           .createEntity()
           .set(Transform{position, glm::vec3(1.0f), glm::vec3(0.0f)})
           .set(Shader{Engine::GetShader("basic")})
           .set(Material{Engine::GetMaterial(materialFile)})
           .set(Mesh{Engine::GetMesh(modelPath)})
           .set(AABB{Engine::GetAABB(modelPath)})
           .set(OutOfBoundsDetector{})
           .build();


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

//secs::Entity mainCamera;

void GameClientImplementation::OnInit()
{
    RegisterClientComponents();
    RegisterClientSystems();


    // Calculate initial values for camera
    glm::vec3 origCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 origDirection = glm::normalize(origCamTarget - Engine::camPos);
    float pitch = glm::degrees(asin(origDirection.y));
    float yaw = glm::degrees(atan2(origDirection.z, origDirection.x));

    /*
    // Create and set up "Main Camera" entity
    mainCamera = secs::EntityBuilder(world)
                 .set(Transform{
                     glm::vec3(0.0f, 5.0f, 10.0f), // Position
                     glm::vec3(1.0f), // Scale
                     glm::vec3(pitch, yaw, 0.0f) // Rotation
                 })
                 .set(Camera{42})
                 .set(Manipulator{2, 50})
                 .build();
    */
    
    // Create and set up "Main Light" entity
    secs::EntityBuilder(world)
        .createEntity()
        .set(Transform{
            glm::vec3(0.0f, 0.0f, 0.0f), // Position
            glm::vec3(0, 0, 0), // Scale
            glm::vec3(15, 0, 0.0f) // Rotation
        })
        .set(Light{glm::vec3(1)})
        .set(Spin{50})
        .build();


    PlaceAsset(glm::vec3(0), "assets/models/ground_plane/mat.povertyMat", "assets/models/ground_plane/ground_plane.fbx");

}

int obDetectorId;

void GameClientImplementation::RegisterClientComponents()
{
    secs::ComponentRegistry::registerType<Spin>("Spin");
    secs::ComponentRegistry::registerType<Manipulator>("Manipulator");
    secs::ComponentRegistry::registerType<HelloWorldComponent>("HelloWorldComponent");
    secs::ComponentRegistry::registerType<OutOfBoundsDetector>("OutOfBoundsDetector");
    secs::ComponentRegistry::registerType<DrawAABB>("DrawAABB");
}

bool isGameOver = true;

void GameClientImplementation::RegisterClientSystems()
{

    int spinTypeId = secs::ComponentRegistry::getID<Spin>();
    int transformTypeId = secs::ComponentRegistry::getID<Transform>();
    int manipulatorTypeId = secs::ComponentRegistry::getID<Manipulator>();


    // SpinTransformSystem
    secs::System spinTransformSystem(
        {spinTypeId, transformTypeId},
        [spinTypeId, transformTypeId](secs::Entity e, secs::World& w)
        {
            auto* spin = w.getComponent<Spin>(e);
            auto* transform = w.getComponent<Transform>(e);
            transform->RotateAroundAxis(glm::vec3(0, 1, 0), spin->rotateSpeed * Engine::deltaTime);
            transform->position.y += Engine::deltaTime;
        }
    );
    Engine::AddSystem(spinTransformSystem);

    // ManipulatorTransformSystem
    secs::System manipulatorTransformSystem(
        {manipulatorTypeId, transformTypeId},
        [manipulatorTypeId, transformTypeId](secs::Entity e, secs::World& w)
        {
            auto* manipulator = w.getComponent<Manipulator>(e);
            auto* transform = w.getComponent<Transform>(e);

            // Movement controls
            if (Engine::IsKeyPressed(SDLK_w)) { transform->position.z -= manipulator->moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_s)) { transform->position.z += manipulator->moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_a)) { transform->position.x -= manipulator->moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_d)) { transform->position.x += manipulator->moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_z)) { transform->position.y -= manipulator->moveSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_x)) { transform->position.y += manipulator->moveSpeed * Engine::deltaTime; }

            // Rotation controls
            if (Engine::IsKeyPressed(SDLK_UP)) { transform->not_rotation.x -= manipulator->rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_DOWN)) { transform->not_rotation.x += manipulator->rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_LEFT)) { transform->not_rotation.y -= manipulator->rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_RIGHT)) { transform->not_rotation.y += manipulator->rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_q)) { transform->not_rotation.z -= manipulator->rotateSpeed * Engine::deltaTime; }
            if (Engine::IsKeyPressed(SDLK_e)) { transform->not_rotation.z += manipulator->rotateSpeed * Engine::deltaTime; }
        }
    );
    Engine::AddSystem(manipulatorTransformSystem);

    auto aaBBCompTypeID = secs::ComponentRegistry::getID<HelloWorldComponent>();
    auto drawAABBId = secs::ComponentRegistry::getID<DrawAABB>();
    // DrawAABBSystem
    secs::System drawAABBSystem(
        {aaBBCompTypeID, transformTypeId, drawAABBId},
        [aaBBCompTypeID, transformTypeId](secs::Entity e, secs::World& w)
        {
            auto* aabb = w.getComponent<AABB>(e);
            const auto* transform = w.getComponent<Transform>(e);
            aabb->DebugDraw(glm::vec3(1, 0, 0), *transform);
        }
    );
    Engine::AddSystem(drawAABBSystem);
                
  // int clickDetector = componentRegistry.getID<ClickDeletor>();
  // int aabbId = componentRegistry.getID<AABB>();
  //// secs::System mouseClick({clickDetector},
  ////                         [clickDetector, transformTypeId, aabbId](secs::Entity e, secs::World w)
  ////                         {
  ////                           
  ////                         });
  // Engine::AddSystem(mouseClick);

    int helloWorldCompId = secs::ComponentRegistry::getID<HelloWorldComponent>();
    secs::System helloWorldSystem({helloWorldCompId, transformTypeId},
                                  [helloWorldCompId, transformTypeId](secs::Entity e, secs::World& w) {
                                      std::cout << "hello world" << std::endl;
                                  });
    Engine::AddSystem(helloWorldSystem);
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
    w.destroyEntity(lastMonkey);
    placedAssets.pop_back();
}


void GameClientImplementation::KeepStuffInSight()
{
    secs::queryChunks<Transform, AABB>(
        world, // Pass the World instance as the first parameter
        [&](const std::vector<secs::Entity>& ents,
            const Transform* transforms,
            const AABB* aabbs,
            const size_t count)
        {
//            auto* cameraTransform = world.getComponent<Transform>(mainCamera);
            bool first = true;
            AABB worldAABB;
            for (size_t i = 0; i < count; ++i)
            {
                auto entityAABB = aabbs[i];
                auto entityTrans = transforms[i];
                if (first)
                {
                    worldAABB = {entityTrans.Apply(entityAABB.min), entityTrans.Apply(entityAABB.max)};
                    first = false;
                }
                else
                {
                    worldAABB.Encapsulate(entityTrans.Apply(entityAABB.min));
                    worldAABB.Encapsulate(entityTrans.Apply(entityAABB.max));
                }
            }

            if (first)
            {
                // No valid AABB found
                return;
            }

            glm::vec3 pos;
            glm::vec3 target;
            worldAABB.AABBView(pos, target, glm::vec3(1.5, 4.5, 1), 5.5f);

            Engine::camLook = target;
            Engine::camPos = pos;
        });
}   

void GameClientImplementation::OnUpdate(float deltaTime)
{
    DrawCross();

    KeepStuffInSight();

    if (Engine::GetKeyUp(SDLK_PERIOD))
    {
        const int amount = std::max(static_cast<int>(placedAssets.size()), 1 );
        const size_t placedCount = amount;
        const auto square = static_cast<size_t>(sqrt(placedCount));
        
        for (int i = 0; i < square; ++i)
        {
            const auto asset = PlaceAsset(glm::vec3(0), "assets/models/monkey/mat.povertyMat", "assets/models/monkey/monkey.fbx");
            placedAssets.push_back(asset);
        }
        int size = placedAssets.size();
        for (int i = 0; i < size; ++i)
        {
            Transform* t = world.getComponent<Transform>(placedAssets[i]);
            size_t row = i / square; // Determine the row
            size_t col = i % square; // Determine the column
            auto pos = glm::vec3(col * 3.0f, 0.0f, row * 3.0f); // Scale to adjust spacing
            // Use spawnPos for placing the asset
            t->position = pos;
        }
        
        std::cout << "Assets: " << placedAssets.size() * 2 << std::endl;
    }
    if (Engine::GetKeyUp(SDLK_COMMA))
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
        Engine::MousePositionToRay(origin, dir);
        
        if (PEPhysics::Raycast(origin, dir, world, hitInfo))
        {
            world.destroyEntity(hitInfo.entity);
            killedMonkeys++;
        }    
    }

    if (!isGameOver)
    {
        secs::queryChunks<Transform, OutOfBoundsDetector>(
            world, // Pass the world instance
            [&](const std::vector<secs::Entity>& entities,
                const Transform* transforms,
                const OutOfBoundsDetector* obDetectors,
                size_t count)
            {
                for (size_t i = 0; i < count; ++i)
                {
                    if (!Engine::IsOnScreen(transforms[i].position))
                    {
                        isGameOver = true;
                        Engine::DisplayMessage("You lost the game");
                        //world = secs::World{}; // Reset the world
                        break; // Exit the loop early since the game is over
                    }
                }
            });
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
