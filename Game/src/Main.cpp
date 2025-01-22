#include <Engine.h>
#include "GameClientImplementation.h"
#include "PEPhysicsTests.h"
#include "Secs.h"

int main()
{
	GameClientImplementation client = {};
	/*
#ifdef PE_DEBUG
	PEPhysicsTests::RunAllTests();
#endif
*/
	Engine::Init(&client);
	return 0;
}