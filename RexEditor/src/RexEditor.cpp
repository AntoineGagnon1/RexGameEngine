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
	Inputs::AddAction("LookRight").AddBinding<MouseInput>(MouseInputType::DeltaX);
	Inputs::AddAction("LookUp").AddBinding<MouseInput>(MouseInputType::DeltaY);

	Cursor::SetCursorMode(CursorMode::Locked);

	auto shader = Shader::FromFile("assets/TestShader.shader");
	
	std::vector<Vector3> vertices = { 
		{ 0.5f, -0.5f, -0.5f},
		{ 0.5f, -0.5f,  0.5f},
		{-0.5f, -0.5f,  0.5f},
		{-0.5f, -0.5f, -0.5f},
		{ 0.5f,  0.5f, -0.5f},
		{ 0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f,  0.5f},
		{-0.5f,  0.5f, -0.5f}
	};

	std::vector<unsigned int> indices = {
		1, 2, 3,
		4, 7, 6,
		4, 5, 1,
		1, 5, 6,
		6, 7, 3,
		4, 0, 3,
		0, 1, 3,
		5, 4, 6,
		0, 4, 1,
		2, 1, 6,
		2, 6, 3,
		7, 4, 3
	};
	auto mesh = Mesh(std::span<Vector3>(vertices), std::span<unsigned int>(indices));


	Scene scene;
	auto e = scene.CreateEntity();
	auto& c = e.AddComponent<MeshRendererComponent>();
	c.mesh = std::make_shared<Mesh>(mesh);
	c.shader = std::make_shared<Shader>(shader); // TODO : use resource(asset) manager

	auto player = scene.CreateEntity();
	player.AddComponent<TransformComponent>().position.z = -3;
	player.AddComponent<CameraComponent>();

	// TODO : In RenderQueue
	//	-Output buffer id ? 
	//	-Textures
	
	const float moveSpeed = 1.0f;
	const float rotationSpeed = 40000.0f;

	float roll = 0.0f, pitch = 0.0f;

	while (!win.ShouldClose())
	{
		Time::StartNewFrame();

		if (Inputs::GetAction("Close").IsDown())
			win.Close();

		auto& playerTransform = player.GetComponent<TransformComponent>();
		playerTransform.position += playerTransform.Forward() * (Inputs::GetAction("MoveForward").GetValue() * moveSpeed * Time::DeltaTime());
		playerTransform.position += playerTransform.Right() * (Inputs::GetAction("MoveRight").GetValue() * moveSpeed * Time::DeltaTime());
		playerTransform.position += playerTransform.Up() * (Inputs::GetAction("MoveUp").GetValue() * moveSpeed * Time::DeltaTime());

		roll += rotationSpeed * -Inputs::GetAction("LookRight").GetValue() * Time::DeltaTime();
		pitch += rotationSpeed * -Inputs::GetAction("LookUp").GetValue() * Time::DeltaTime();

		pitch = Scalar::Clamp(pitch, -89.999f, 89.999f);

		playerTransform.rotation = Quaternion::AngleAxis(roll, Directions::Up);
		playerTransform.rotation.Rotate(pitch, playerTransform.Right());

		RenderApi::ClearColorBit();

		ForwardRenderer::RenderScene(scene, player.GetComponent<CameraComponent>()); // TODO : Lights

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;
}