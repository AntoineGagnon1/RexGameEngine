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


	Inputs::AddAction("Close").AddBinding<KeyboardInput>(KeyCode::Escape);
	Inputs::AddAction("MoveForward").AddBinding<KeyboardInput>(KeyCode::W, KeyCode::S);
	Inputs::AddAction("MoveRight").AddBinding<KeyboardInput>(KeyCode::D, KeyCode::A);
	Inputs::AddAction("MoveUp").AddBinding<KeyboardInput>(KeyCode::Space, KeyCode::LeftShift);


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
	camTransform.position.z = -3;

	// TODO : In RenderQueue
	//	-Output buffer id ? 
	//	-Textures
	
	// TODO : mouse inputs

	const float moveSpeed = 1.0f;

	while (!win.ShouldClose())
	{
		Time::StartNewFrame();

		if (Inputs::GetAction("Close").IsDown())
			win.Close();

		auto& transform = camEntity.GetComponent<TransformComponent>();
		transform.position.z += Inputs::GetAction("MoveForward").GetValue() * moveSpeed * Time::DeltaTime();
		transform.position.x += Inputs::GetAction("MoveRight").GetValue() * moveSpeed * Time::DeltaTime();
		transform.position.y += Inputs::GetAction("MoveUp").GetValue() * moveSpeed * Time::DeltaTime();

		RenderApi::ClearColorBit();

		ForwardRenderer::RenderScene(scene, cam); // TODO : Lights

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;
}