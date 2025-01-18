#include "GameClientImplementation.h"
#include "systems/ManipulatorSystem.h"
#include "systems/TransformSystem.h"
#include "systems/SpinSystem.h"

int add(int a, int b) {
    return a + b;
}
std::vector<flecs::entity> placedAssets;

::flecs::entity PlaceAsset(
    flecs::world& ecs,
    const glm::vec3& position,
    const glm::vec3& color,
    const std::string& entityName,
    const std::string& materialFolder,
    const std::string& modelPath)
{
    return Engine::CreateEntity(
        entityName.c_str(),
                Transform{ glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f) },
                Shader{Engine::GetShader("basic")},
                Material{Engine::GetMaterial(materialFolder)},
                Mesh{Engine::GetMesh(modelPath)});
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

void PopulateMonkeys(flecs::world& ecs) {
    const float spacing = 2.5f; 
    const int count = 20;      
    for (int i = 0; i < count; ++i) {
        float x = (i % 5) * spacing - 5.0f;
        float z = (i / 5) * spacing - 5.0f;
        PlaceAsset(ecs, glm::vec3(x, 0.0f, z), GetRandomColor(), "Monkey" + std::to_string(i + 1), "assets/models/monkey/", "assets/models/monkey/monkey.fbx");
    }
}

struct HelloWorldComponent
{
    int number = 42;
};

void GameClientImplementation::OnInit()
{
    throw std::exception("this shit needs to be implemented");
  //  glm::vec3 origCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
  //  glm::vec3 origDirection = glm::normalize(origCamTarget - Engine::camPos);
  //  float pitch = glm::degrees(asin(origDirection.y)); 
  //  float yaw = glm::degrees(atan2(origDirection.z, origDirection.x));
//
  //  ecs.entity("Main Camera")
  //     .set<Transform>({
  //         glm::vec3(0.0f, 5.0f, 10.0f),
  //         glm::vec3(1.0f),
  //         glm::vec3(pitch, yaw, 0.0f)
  //     })
  //     .set<Camera>({42})
  //     .set<Manipulator>({2, 50});
  //  ecs.entity("Main Light")
  //     .set<Transform>({
  //         glm::vec3(0.0f, 0.0f, 0.0f),
  //         glm::vec3(0, 0, 0),
  //         glm::vec3(15, 0, 0.0f)
  //     })
//
  //     .set<Light>({glm::vec3(1.0f, 1.0f, 1.0f)})
  //     .set<Spin>({50});
//
  //  TransformSystem::RegisterSystem(ecs);
  //  SpinSystem::RegisterSystem(ecs);
  //  RenderSystem::RegisterSystem(ecs);
  //   

}

void GameClientImplementation::OnUpdate(float deltaTime)
{
    if (Engine::IsKeyPressed(SDLK_PERIOD))
    {
        Engine::CreateEntity("hello", HelloWorldComponent{42});
        
        auto asset = PlaceAsset(ecs, glm::vec3(placedAssets.size(), 0, 0), GetRandomColor(),"monkey" + std::to_string(placedAssets.size()), "assets/models/monkey", "assets/models/monkey/monkey.fbx");
        //placedAssets.push_back(asset);
        std::cout << "Assets: " << placedAssets.size() << std::endl;
    }
    if (Engine::IsKeyPressed(SDLK_COMMA))
    {
        if (!placedAssets.empty())
        {
            auto lastMonkey = placedAssets.back();
            lastMonkey.destruct();
            placedAssets.pop_back();
            std::cout << "Assets " << placedAssets.size() << std::endl;
        }
        else
        {
            std::cout << "Scene "
                "empty" << std::endl;
        }
    }
}

void GameClientImplementation::OnShutdown()
{
}
