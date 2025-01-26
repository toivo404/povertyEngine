#include "GameClientImplementation.h"
#include <algorithm>
#include "imgui.h"
#include "systems/TransformSystem.h"
#include "MathUtils.h"
#include "ModelFileLoader.h"
#include "parseLevelFile.h"

struct Spin
{
    float rotateSpeed = 50.0f; // Rotation speed in degrees per second
};

struct HelloWorldComponent
{
    int number = 42;
};

struct Manipulator
{
    float moveSpeed = 2.0f;
    float rotateSpeed = 50.0f;
};
// ---------------- NEW COMPONENT --------------------
struct ModelViewer
{
    float rotateSpeed = 50.0f; 
    float scaleSpeed  = 1.0f;
};

struct OutOfBoundsDetector
{
};


struct Car
{
    float speed = 0;
    float maxSpeed = 100;

    bool accelerating = false;

    // Additional turning fields
    float turnSpeed = 50.0f;
    float turnInput = 0.0f;
};


struct GroundMesh
{
    std::vector<glm::vec3>* vertices;
    std::vector<unsigned int>* indices;
};

int add(int a, int b)
{
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
           .build();
}

secs::Entity PlaceCar(secs::World* world, glm::vec3 position)
{
    // Create a new entity
    return secs::EntityBuilder(*world)
           .createEntity()
           .set(Transform{position, glm::vec3(1.0f), glm::vec3(0.0f)})
           .set(Shader{Engine::GetShader("basic")})
           .set(Mesh{Engine::GetMesh("assets/models/testcar/car.fbx")})
           .set(AABB{Engine::GetAABB("assets/models/testcar/car.fbx")})
           .set(Material{Engine::GetMaterial("assets/materials/ritari_palette/colorpalette.povertyMat")})
           .set(Car{})
           .build();
}

float GetRandomValue()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

glm::vec3 GetRandomColor()
{
    const glm::vec3 baseColor(0.6f, 0.2f, 0.8f);
    return baseColor + glm::vec3(
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

secs::Entity pcCar;

void GameClientImplementation::OnInit()
{
    RegisterClientComponents();
    RegisterClientSystems();

    

    cameraLookPosEntity = secs::EntityBuilder(world)
                          .createEntity()
                          .set(Transform{Engine::camLook})
                          .set(Manipulator{2.0f, 30.0f})
                          .build();

    // Create and set up "Main Light" entity
    //  secs::EntityBuilder(world)
    //      .createEntity()
    //      .set(Transform{
    //          glm::vec3(0.0f, 0.0f, 0.0f), // Position
    //          glm::vec3(0, 0, 0), // Scale
    //          glm::vec3(0, 0, 180.0f) // Rotation
    //      })
    //      .set(Light{glm::vec3(1)})
    //      .build();
    Engine::lightDir = glm::vec3(0, -1, 0);

 
   
    
}

int obDetectorId;

void GameClientImplementation::RegisterClientComponents()
{
    secs::ComponentRegistry::registerType<Spin>("Spin");
    secs::ComponentRegistry::registerType<Manipulator>("Manipulator");
    secs::ComponentRegistry::registerType<HelloWorldComponent>("HelloWorldComponent");
    secs::ComponentRegistry::registerType<OutOfBoundsDetector>("OutOfBoundsDetector");
    secs::ComponentRegistry::registerType<Car>("Car");
    secs::ComponentRegistry::registerType<GroundMesh>("GroundMesh");
    secs::ComponentRegistry::registerType<ModelViewer>("ModelViewer");
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
    auto carTypeId = secs::ComponentRegistry::getID<Car>();
    auto aaBbTypeId = secs::ComponentRegistry::getID<AABB>();
    secs::System carSystem(
        {carTypeId, transformTypeId, aaBbTypeId},
        [&](secs::Entity carEntity, secs::World& w)
        {
            auto car = w.getComponent<Car>(carEntity);
            auto carTrans = w.getComponent<Transform>(carEntity);

            // 1) Move the car in its forward direction by current speed
            //    We'll treat speed as units per second. 
            carTrans->position += carTrans->GetDirection() * car->speed * Engine::deltaTime;

            // 2) Apply turning *based on speed*
            //    - The faster you go, the more sensitive (or the other way around if you prefer).
            //    - Example: turnAngle = turnInput * turnSpeed * (car->speed / maxSpeed).
            float normalizedSpeed = car->speed / car->maxSpeed;
            float turnAngle = car->turnInput
                * car->turnSpeed
                * normalizedSpeed
                * Engine::deltaTime; // scale by deltatime
            if (fabs(car->turnInput) > 0.001f && fabs(car->speed) > 0.01f)
            {
                carTrans->RotateAroundAxis(glm::vec3(0, 1, 0), turnAngle);
            }

            // 3) If not accelerating, apply friction
            if (!car->accelerating)
            {
                // MoveTowards is a linear movement, you might want to do something else for friction
                car->speed = MathUtils::MoveTowards(car->speed, 0.0f, Engine::deltaTime * 2.0f);
            }
            car->turnInput = glm::mix(car->turnInput, 0.0f, 3 * Engine::deltaTime);
            // Reset accelerating flag for next frame
            car->accelerating = false;
            auto* carAabb = w.getComponent<AABB>(carEntity);

            secs::queryChunks<Transform, AABB>(
                world, // Pass the World instance as the first parameter
                [carEntity, car, carAabb, carTrans](const std::vector<secs::Entity>& ents,
                    const Transform* transforms,
                    const AABB* aabbs,
                    const size_t count)
                {
                    for (size_t i = 0; i < count; i++)
                    {
                        if (ents[i].id == carEntity.id)
                            continue;
                        bool hit = PEPhysics::CheckAABBOverlap(aabbs[i], transforms[i], *carAabb, *carTrans);
                        if (hit)
                            car->speed = glm::sign(car->speed * -1) * car->speed * 0.2f;  
                    }
                });
        }
    );
    Engine::AddSystem(carSystem);

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
            if (Engine::IsKeyPressed(SDLK_UP))
            {
                transform->RotateAroundAxis(glm::vec3(-1, 0, 0), Engine::deltaTime);
            }
            if (Engine::IsKeyPressed(SDLK_DOWN))
            {
                transform->RotateAroundAxis(glm::vec3(1, 0, 0), Engine::deltaTime);
            }
            if (Engine::IsKeyPressed(SDLK_LEFT))
            {
                transform->RotateAroundAxis(glm::vec3(0, -1, 0), Engine::deltaTime);
            }
            if (Engine::IsKeyPressed(SDLK_RIGHT))
            {
                transform->RotateAroundAxis(glm::vec3(0, 1, 0), Engine::deltaTime);
            }
            if (Engine::IsKeyPressed(SDLK_q))
            {
                transform->RotateAroundAxis(glm::vec3(0, 0, -1), Engine::deltaTime);
            }
            if (Engine::IsKeyPressed(SDLK_e))
            {
                transform->RotateAroundAxis(glm::vec3(0, 0, 1), Engine::deltaTime);
            }
        }
    );
    Engine::AddSystem(manipulatorTransformSystem);

    // int clickDetector = componentRegistry.getID<ClickDeletor>();
    // int aabbId = componentRegistry.getID<AABB>();
    //// secs::System mouseClick({clickDetector},
    ////                         [clickDetector, transformTypeId, aabbId](secs::Entity e, secs::World w)
    ////                         {
    ////                           
    ////                         });
    // Engine::AddSystem(mouseClick);

    // ---------------- NEW: Model Viewer System ----------------
    int modelViewerTypeId = secs::ComponentRegistry::getID<ModelViewer>();
    secs::System modelViewerSystem(
        {modelViewerTypeId, transformTypeId},
        [modelViewerTypeId, transformTypeId](secs::Entity e, secs::World& w)
        {
            auto* viewer    = w.getComponent<ModelViewer>(e);
            auto* transform = w.getComponent<Transform>(e);

            // Rotate around Y with Left/Right
            if (Engine::IsKeyPressed(SDLK_LEFT))
            {
                transform->RotateAroundAxis(glm::vec3(0, 1, 0), Engine::deltaTime*100);
            }
            if (Engine::IsKeyPressed(SDLK_RIGHT))
            {
                transform->RotateAroundAxis(glm::vec3(0, -1, 0), Engine::deltaTime*100);
            }
            // Rotate around X with Up/Down
            if (Engine::IsKeyPressed(SDLK_UP))
            {
                transform->RotateAroundAxis(glm::vec3(1, 0, 0), Engine::deltaTime*100);
            }
            if (Engine::IsKeyPressed(SDLK_DOWN))
            {
                transform->RotateAroundAxis(glm::vec3(-1, 0, 0), Engine::deltaTime*100);
            }

            // Scale up/down with Numpad +/-
            if (Engine::IsKeyPressed(SDLK_KP_PLUS))
            {
                // Uniform scale
                transform->scale += glm::vec3(viewer->scaleSpeed) * Engine::deltaTime;
            }
            if (Engine::IsKeyPressed(SDLK_KP_MINUS))
            {
                transform->scale -= glm::vec3(viewer->scaleSpeed) * Engine::deltaTime;
                // OPTIONAL: clamp scale so we don’t go negative
                transform->scale = glm::max(transform->scale, glm::vec3(0.01f));
            }
        }
    );
    
    Engine::AddSystem(modelViewerSystem);
    int helloWorldCompId = secs::ComponentRegistry::getID<HelloWorldComponent>();
    secs::System helloWorldSystem({helloWorldCompId, transformTypeId},
                                  [helloWorldCompId, transformTypeId](secs::Entity e, secs::World& w)
                                  {
                                      std::cout << "hello world" << std::endl;
                                  });
    Engine::AddSystem(helloWorldSystem);
}


void GameClientImplementation::DrawCross(glm::vec3 pos)
{
    Engine::DebugDrawLine(pos + glm::vec3(0.0, 1, 0), pos + glm::vec3(0.0, -1, 0), glm::vec3(0.0, 0, 0), 0.1f);
    Engine::DebugDrawLine(pos + glm::vec3(-1, 0, 0), pos + glm::vec3(1, 0, 0), glm::vec3(0.0, 0, 0), 0.1f);
    Engine::DebugDrawLine(pos + glm::vec3(0, 0, 1), pos + glm::vec3(0, 0, -1), glm::vec3(0.0, 0, 0), 0.1f);
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

bool keepStuffInSight;

void GameClientImplementation::KeepStuffInSight()
{
    if (!keepStuffInSight)
        return;
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


glm::vec3 GameClientImplementation::GetCameraOffset()
{
    return  glm::vec3(0, 60, 5) ;
}

void GameClientImplementation::PlayerControls()
{
    auto* pcCarTrans = world.getComponent<Transform>(pcCar);
    auto* car = world.getComponent<Car>(pcCar);
    //auto pos = world.getComponent<Transform>(cameraLookPosEntity)->position;


    // camTrans->not_rotation  = pcCarTrans->not_rotation;
    // camTrans->AddRotation(glm::vec3(90, 0, 0));
    // 
    //   camTrans->LookAt(pcCarTrans->position);
    //Engine::camLook = Engine::camPos + glm::vec3(1, -1, 0);

    //Engine::camLook = world.getComponent<Transform>(cameraLookPosEntity)->position;

    if (isGameOver)
        return;
    if (!keepStuffInSight)
    {
        Engine::camLook = pcCarTrans->position;
        Engine::camPos = glm::mix(Engine::camPos ,pcCarTrans->position + GetCameraOffset(), fabs(car->speed)*Engine::deltaTime);
    }


    Engine::DebugStat("turnInput", std::to_string(  car->turnInput ) );
    Engine::DebugStat("turnSpeed", std::to_string( car->turnSpeed ));
    // reference values
    // 10 =  1
    // 100 =  2
    // 200 =  2.30103
    // 300 =  2.47712
    float logSpeed = car->speed > 10 ? log10f(car->speed) : 1;
    float speedIncrement = 10.0f * Engine::deltaTime;
    if (logSpeed > 1)
        speedIncrement /= logSpeed;

    keepStuffInSight = Engine::GetKey(SDLK_l);

    if (Engine::GetKey(SDLK_UP))
    {
        if (car->speed < 0.0f) // Braking
        {
            float brakingFactor = 5.0f * Engine::deltaTime;
            car->speed = glm::mix(car->speed, 0.0f, brakingFactor);
        }
        else // Accelerating normally
        {
            car->speed += speedIncrement;
            car->accelerating = true;
        }
    }

    if (Engine::GetKey(SDLK_DOWN))
    {
        if (car->speed > 0.0f) // Braking
        {
            float brakingFactor =  5.0f * Engine::deltaTime;
            car->speed = glm::mix(car->speed, 0.0f, brakingFactor);
        }
        else // Reversing normally
        {
            car->speed -= speedIncrement;
            car->accelerating = true;
        }
    }


 
    float speedTurningFactor = logSpeed * 2;
    if (Engine::GetKey(SDLK_LEFT))
    {
        car->turnInput += (-100.0f * Engine::deltaTime) / speedTurningFactor;
    }
    else if (Engine::GetKey(SDLK_RIGHT))
    {
        car->turnInput += (100.0f * Engine::deltaTime) / speedTurningFactor;
    }
}

int currentModelIndex = -1;
secs::Entity prev;

std::string GameClientImplementation::LoadModels()
{
    return ModelFileLoader::Load(std::string(Engine::baseFilePath) + "assets/models.json", models);
}

void GameClientImplementation::CycleModels()
{
    if (currentModelIndex != -1)
        world.destroyEntity(prev);
    auto msg = LoadModels();
    Engine::DebugStat("msg",msg);
    Engine::DebugStat("model no ", std::to_string(currentModelIndex) + " / " + std::to_string(models.size())) ;
    
    auto model = models[currentModelIndex % models.size()];
    auto placedModel = PlaceAsset(glm::vec3(0),  model.textureFile, model.modelFile);
    world.addComponent(placedModel, ModelViewer{ 10, 1});
    auto aabb = world.getComponent<AABB>(placedModel);
    Engine::camPos = 2.0f * aabb->max;
    Engine::DebugStat("viewed model min", PositionString(aabb->min));
    Engine::DebugStat("viewed model max", PositionString(aabb->max));
    prev = placedModel;
    currentModelIndex++;
}

void GameClientImplementation::StartGame()
{
    glm::vec3 spawnPos(10.0f, 0.0f, 0.0f);
    pcCar = PlaceCar(&world, spawnPos);
    Engine::camPos = spawnPos + GetCameraOffset();
    auto levelProps = parseLevelFile("assets/sectors/monkey.sector");
    LoadModels();
    for (auto item : models)
    {
        for (auto prop : levelProps)
        {
            if (prop.first == item.name)
            {
                secs::EntityBuilder(world)
                    .createEntity()
                    .set(prop.second)
                    .set(Shader{Engine::GetShader("basic")})
                    .set(Material{Engine::GetMaterial(item.textureFile)})
                    .set(Mesh{Engine::GetMesh(item.modelFile)})
                    .set(AABB{Engine::GetAABB(item.modelFile)})
                    .build();
            }
        }
    } 
    
        
    isGameOver = !isGameOver;
}

void GameClientImplementation::OnUpdate(float deltaTime)
{
    Engine::DebugStat("isGameOver", std::to_string(isGameOver));

    KeepStuffInSight();
    PlayerControls();

    if (isGameOver && Engine::GetKeyUp(SDLK_SPACE))
    {
        CycleModels();   
    }
    
    if (Engine::GetKeyUp(SDLK_PERIOD))
    {
        const int amount = std::max(static_cast<int>(placedAssets.size()), 1);
        const size_t placedCount = amount;
        const auto square = static_cast<size_t>(sqrt(placedCount));

        for (int i = 0; i < square; ++i)
        {
            const auto asset = PlaceAsset(glm::vec3(0), "assets/models/monkey/mat.povertyMat",
                                          "assets/models/monkey/monkey.fbx");
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
                for (int i = 0; i < amount; ++i)
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
    if (isGameOver)
    {
        secs::queryChunks<Transform, AABB>(
            world, // Pass the World instance as the first parameter
            [&](const std::vector<secs::Entity>& ents,
                const Transform* transforms,
                const AABB* aabbs,
                const size_t count)
            {
                for (size_t i = 0; i < count; ++i)
                {
                    auto* aabb = world.getComponent<AABB>(ents[i]);
                    const auto* transform = world.getComponent<Transform>(ents[i]);
                    aabb->DebugDraw(glm::vec3(1, 0, 0), *transform);
                }
            });
      
    }
    if (isGameOver && Engine::GetKeyUp(SDLK_BACKSPACE) && Engine::GetKey(SDLK_LSHIFT))
    {
        StartGame();
    }

    // if(Engine::GetMouseButtonUp(1))
    // {
    //     std::cout<<"Click" << std::endl;
    //     glm::vec3 origin;
    //     glm::vec3 dir;
    //     PEPhysicsHitInfo hitInfo;
    //     Engine::MousePositionToRay(origin, dir);
    //     
    //     if (PEPhysics::Raycast(origin, dir, world, hitInfo))
    //     {
    //         world.destroyEntity(hitInfo.entity);
    //     }    
    // }
}

void GameClientImplementation::OnShutdown()
{
}


