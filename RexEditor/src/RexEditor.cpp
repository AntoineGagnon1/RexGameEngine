#include <iostream>

#include <RexEngine.h>
#include <span>

using namespace RexEngine;

struct TempComponent 
{
	int someData;

	TempComponent(int data) : someData(data)
	{}
};


int main()
{
	Scene scene;
	auto e = scene.CreateEntity();

	e.AddComponent<TempComponent>(10);
	RE_ASSERT(e.GetComponent<TempComponent>().someData == 10, "Component failed");
	RE_ASSERT(e.RemoveComponent<TempComponent>(), "Remove component failed");
	RE_ASSERT(!e.HasComponent<TempComponent>(), "Has component should be false");
	scene.DestroyEntity(e);

	RE_ASSERT(!e, "Entity should not be valid");


	Window win("Test window", 640, 480);
	win.MakeActive();
	
	win.SetResizeCallback([](Vector2Int size) {
		RenderApi::SetViewportSize(size);
	});

	auto shader = Shader::FromFile("assets/TestShader.shader");

	std::vector<Vector3> vertices = { Vector3{-0.75f,-0.75f,1}, Vector3{0,0.75f,1}, Vector3{0.75f,-0.75f,1} };
	std::vector<Vector3> normals = { Vector3{0,0,0}, Vector3{0,0,0}, Vector3{0,0,0} };
	std::vector<unsigned int> indices = { 0,1,2 };
	auto mesh = Mesh(std::span<Vector3>(vertices), std::span<unsigned int>(indices), std::span<Vector3>(normals));

	Inputs::AddAction("Close").AddBinding<KeyboardInput>(KeyCode::Escape);

	
	while (!win.ShouldClose())
	{
		if (Inputs::GetAction("Close").IsDown())
			win.Close();

		RenderApi::ClearColorBit();

		shader.Bind();
		mesh.Bind();
		RenderApi::DrawElements(3);

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;
}