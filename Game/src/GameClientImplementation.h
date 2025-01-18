#pragma once
#include "GameClient.h"
#include "Engine.h"
#include <iostream>

class GameClientImplementation : public GameClient {
public:
    void OnInit() override;

    void OnUpdate(float deltaTime) override;

    void OnShutdown() override;
};