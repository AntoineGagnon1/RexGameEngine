#include <iostream>

#include <RexEngine.h>

struct TempComponent 
{
	int someData;

	TempComponent(int data) : someData(data)
	{}
};

int main()
{
	RE_ASSERT(true, "This assert was false");

	RE_LOG_DEBUG("Debug {} {}", 12, "test");
	RE_LOG_INFO("Hello {}", 123);
	RE_LOG_WARN("Warn {}", "warn");
	RE_LOG_ERROR("ERROR {}", 3.0f);


	RexEngine::Scene scene;
	auto e = scene.CreateEntity();

	e.AddComponent<TempComponent>(10);
	RE_ASSERT(e.GetComponent<TempComponent>().someData == 10, "Component failed");
	RE_ASSERT(e.RemoveComponent<TempComponent>(), "Remove component failed");
	RE_ASSERT(!e.HasComponent<TempComponent>(), "Has component should be false");
	scene.DestroyEntity(e);


	RE_ASSERT(!e, "Entity should not be valid");
	return 0;
}