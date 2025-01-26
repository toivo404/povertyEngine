#pragma once
#include "GameClient.h"
#include "Engine.h"
#include <iostream>

#include "ModelFileLoader.h"

class GameClientImplementation : public GameClient {
public:
    secs::Entity PlaceAsset(const glm::vec3& position, const std::string& materialFolder, const std::string& modelPath);
    void OnInit() override;
    void RegisterClientComponents();
    void RegisterClientSystems();

    void DrawCross(glm::vec3 pos);
    void KeepStuffInSight();

    void PlayerControls();
    std::string LoadModels();
    void CycleModels();
    void StartGame();
    void OnUpdate(float deltaTime) override;

    void OnShutdown() override;
    secs::Entity cameraLookPosEntity;
    secs::Entity groundEntity;
    std::vector<ModelData> models;
};
