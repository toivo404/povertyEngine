#ifndef GAME_CLIENT_H  // If GAME_CLIENT_H is not defined
#define GAME_CLIENT_H  // Define GAME_CLIENT_H
#include "GameClient.h"
#include "secs.h"
class PE_API GameClient {
public:
    virtual void OnInit() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnShutdown() = 0;
    
    secs::World world;
};

#endif // GAME_CLIENT_H
