#include <Engine.h>
#include <iostream>

#include "GameClientImplementation.h"
#include "MeshCache.h"
#include "ShaderLoader.h"
#include "systems/MaterialSystem.h"

int main()
{
	GameClientImplementation client = {};
	Engine::Init(&client);
	return 0;
}