#include <Engine.h>
#include <iostream>

#include "GameClientImplementation.h"
#include "MeshCache.h"
#include "Secs.h"
#include "ShaderLoader.h"
#include "SecsTest.h"
#include "..\..\PovertyEngine\src\PEPhysicsTests.cpp"
struct MyHelloWorldComponent
{
	int number = 42;
	// Provide a constructor that accepts an int
	MyHelloWorldComponent() = default;
	MyHelloWorldComponent(int n)
		: number(n)
	{
	}
};

int main()
{
	std::cout << "hello "<< std::endl;
	GameClientImplementation client = {};
	secs::World testWorld = {};
#ifdef PE_DEBUG
	RunAllSecsTests(testWorld);
	PEPhysicsTests::RunAllTests();
#endif
	
	Engine::Init(&client);
	
	//// Create a couple of entities
	//secs::Entity e1 = world.addEntity("EntityOne");
	//secs::Entity e2 = world.addEntity("EntityTwo");
//
	//// Add a component to them
	//world.addComponent<MyHelloWorldComponent>(e1, 123);
	//world.addComponent<MyHelloWorldComponent>(e2, 999);
//
	//// Define a system that operates on MyHelloWorldComponent
	//secs::System<MyHelloWorldComponent> printSystem(
	//	[](secs::Entity e, MyHelloWorldComponent& hwc, secs::World& w){
	//		std::cout << "System sees entity " << e
	//				  << " with component.number = " << hwc.number << "\n";
	//		// Maybe do some update:
	//		hwc.number += 1; 
	//	}
	//);
//
	//// We can store systems in a vector
	//std::vector<secs::SystemBase*> systems;
	//systems.push_back(&printSystem);
//
	//// Run one 'tick' of all systems
	//secs::progress(world, systems);
//
	//// Check that the system mutated the component
	//auto& c1 = world.getComponent<MyHelloWorldComponent>(e1);
	//auto& c2 = world.getComponent<MyHelloWorldComponent>(e2);
	//std::cout << "After system update:\n";
	//std::cout << "Entity " << e1 << " number=" << c1.number << "\n";
	//std::cout << "Entity " << e2 << " number=" << c2.number << "\n";
//
	
	return 0;
}