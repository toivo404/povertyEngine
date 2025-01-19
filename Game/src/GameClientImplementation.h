#pragma once
#include "GameClient.h"
#include "Engine.h"
#include <iostream>

class GameClientImplementation : public GameClient {
public:
    secs::Entity PlaceAsset(const glm::vec3& position, const std::string& materialFolder, const std::string& modelPath);
    void OnInit() override;
    void RegisterClientComponents();
    void GameOver();
    void RegisterClientSystems();

    void OnUpdate(float deltaTime) override;

    void OnShutdown() override;
};