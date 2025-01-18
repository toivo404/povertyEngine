#ifndef GAME_CLIENT_H  // If GAME_CLIENT_H is not defined
#define GAME_CLIENT_H  // Define GAME_CLIENT_H
#include "GameClient.h"
#include "flecs.h"
class GameClient {
public:
    virtual void OnInit() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnShutdown() = 0;
    flecs::world ecs;

};

#endif // GAME_CLIENT_H
