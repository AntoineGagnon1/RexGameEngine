#include <iostream>

#include <RexEngine.h>
#include <span>

using namespace RexEngine;


int main()
{
	Window win("Test window", 640, 480);
	win.MakeActive();
	
	win.SetResizeCallback([](Vector2Int size) {
		RenderApi::SetViewportSize(size);
	});


	auto shader = Shader::FromFile("assets/TestShader.shader");
	
	std::vector<Vector3> vertices = { Vector3{-0.75f,-0.75f,1}, Vector3{0,0.75f,1}, Vector3{0.75f,-0.75f,1} };
	std::vector<unsigned int> indices = { 0,1,2 };
	auto mesh = Mesh(std::span<Vector3>(vertices), std::span<unsigned int>(indices));


	Scene scene;
	auto e = scene.CreateEntity();
	auto& c = e.AddComponent<MeshRendererComponent>();
	c.mesh = std::make_shared<Mesh>(mesh);
	c.shader = std::make_shared<Shader>(shader); // TODO : use resource(asset) manager

	auto camEntity = scene.CreateEntity();
	auto& cam = camEntity.AddComponent<CameraComponent>();
	auto& camTransform = camEntity.AddComponent<TransformComponent>();
	camTransform.position.z = -1;

	Inputs::AddAction("Close").AddBinding<KeyboardInput>(KeyCode::Escape);
	
	while (!win.ShouldClose())
	{
		if (Inputs::GetAction("Close").IsDown())
			win.Close();

		RenderApi::ClearColorBit();

		ForwardRenderer::RenderScene(scene, cam); // TODO : Lights

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;
}