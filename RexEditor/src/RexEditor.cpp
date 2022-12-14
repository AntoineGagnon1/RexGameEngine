#include "REDPch.h"

#include "ui/UI.h"
#include "ui/MenuBar.h"
#include "panels/PanelManager.h"
#include "panels/SceneView.h"
#include "project/ProjectManager.h"

int main()
{
	using namespace RexEngine;
	using namespace RexEditor;

	Window win("RexEditor", 1280, 720, 8);
	win.MakeActive();

	win.SetResizeCallback([](Vector2Int size) {
		RenderApi::SetViewportSize(size);
	});

	EditorEvents::OnEditorStart().Dispatch();
	EngineEvents::OnEngineStart().Dispatch();

	ScriptEngine::LoadAssembly(Dirs::ScriptDir / "Editor" / "RexEditorScript.dll");

	EditorEvents::OnEditorStarted().Dispatch();

	PanelManager::RegisterPanel<SceneViewPanel>("Scene View");

	// TODO : Convert guid to struct based design ? (push on create/pop on destroy ?)

	// Temp to save some time while testing
	ProjectManager::Load("../../../RexEditor/Projects/TestProject/TestProject.rexengine");

	auto f = ScriptEngine::GetManagedFunction<void, Guid>("RexEditor.Class1", "Test");
	auto scene = SceneManager::CreateScene();
	SceneManager::SetCurrentScene(scene);
	auto e = scene.CreateEntity();
	auto e2 = scene.CreateEntity();
	f(e.GetGuid());
	e2.GetComponent<TransformComponent>().parent = e;
	e2.AddComponent<CameraComponent>();
	e2.AddComponent<MeshRendererComponent>();
	auto skybox = e.AddComponent<SkyboxComponent>();
	Guid shaderGuid = Guid::Empty;
	*(uint64_t*)(&shaderGuid) += 1;
	//AssetManager::AddAsset<Shader>(shaderGuid, "assets/skybox/Skybox.shader");

	skybox.shader = AssetManager::GetAsset<Shader>(shaderGuid);
	//e.AddComponent<MeshRendererComponent>();
	f(e.GetGuid());

	//stream.clear();
	//scene2.Serialize(test);
	Timer editorFrameTime;
	editorFrameTime.Start();
	while (!win.ShouldClose())
	{
		float deltaTime = (float)editorFrameTime.ElapsedSeconds();
		editorFrameTime.Restart();
		EngineEvents::OnPreUpdate().Dispatch();
		UI::NewFrame();
		
		UI::MenuBar::DrawMenuBar();

		EngineEvents::OnUpdate().Dispatch();
		PanelManager::RenderPanels(deltaTime);

		UI::RenderUI();

		win.SwapBuffers();
	}
	
	EngineEvents::OnEngineStop().Dispatch();
	EditorEvents::OnEditorStop().Dispatch();

	return 0;
}

/*
	Inputs::AddAction("Close").AddBinding<KeyboardInput>(KeyCode::Escape);
	Inputs::AddAction("MoveForward").AddBinding<KeyboardInput>(KeyCode::W, KeyCode::S);
	Inputs::AddAction("MoveRight").AddBinding<KeyboardInput>(KeyCode::D, KeyCode::A);
	Inputs::AddAction("MoveUp").AddBinding<KeyboardInput>(KeyCode::Space, KeyCode::LeftShift);
	Inputs::AddAction("LookRight").AddBinding<MouseInput>(MouseInputType::DeltaX);
	Inputs::AddAction("LookUp").AddBinding<MouseInput>(MouseInputType::DeltaY);

	Cursor::SetCursorMode(CursorMode::Locked);

	auto shader = Shader::FromFile("assets/TestShader.shader");
	shader->SetUniformVector3("albedo", Vector3(1.0f, 0.0f, 0.0f));
	shader->SetUniformFloat("metallic", 0.5f);
	shader->SetUniformFloat("roughness", 0.1f);
	shader->SetUniformFloat("ao", 1.0f);
	shader->SetUniformInt("irradianceMap", 1);
	shader->SetUniformInt("prefilterMap", 2);
	shader->SetUniformInt("brdfLUT", 3);

	auto skyboxShader = Shader::FromFile("assets/skybox/Skybox.shader");
	skyboxShader->SetUniformInt("skybox", 0);
	auto skyboxMap = PBR::FromHDRI("assets/skybox/env.hdr", Vector2Int(1024, 1024));

	auto skyboxIrradiance = PBR::CreateIrradianceMap(*skyboxMap, Vector2Int(32, 32), 0.025f);

	auto skyboxPrefilter = PBR::CreatePreFilterMap(*skyboxMap, Vector2Int(128,128));

	auto pbrLUT = PBR::CreateBRDFLut(Vector2Int(512, 512));

	RenderApi::SetActiveTexture(0);
	skyboxMap->Bind();

	RenderApi::SetActiveTexture(1);
	skyboxIrradiance->Bind();
	RenderApi::SetActiveTexture(2);
	skyboxPrefilter->Bind();
	RenderApi::SetActiveTexture(3);
	pbrLUT->Bind();

	RenderApi::SetActiveTexture(0);

	Scene scene;
	auto light = scene.CreateEntity();
	auto& lightTransform = light.AddComponent<TransformComponent>();
	lightTransform.position = Vector3(10, 10, -10);
	lightTransform.scale = Vector3(0.1f, 0.1f, 0.1f);
	auto& lightMesh = light.AddComponent<MeshRendererComponent>();
	lightMesh.shader = shader;
	lightMesh.mesh = Shapes::GetSphereMesh();

	light.AddComponent<SkyboxComponent>().shader = skyboxShader;

	for (int x = 0; x < 10; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			auto sphere = scene.CreateEntity();
			sphere.AddComponent<TransformComponent>().position = Vector3(x * 2, y * 2, 0);

			auto& meshRenderer = sphere.AddComponent<MeshRendererComponent>();
			meshRenderer.cullingMode = RenderApi::CullingMode::Front;
			meshRenderer.mesh = Shapes::GetSphereMesh();
			meshRenderer.shader = shader;
		}
	}

	auto player = scene.CreateEntity();
	player.AddComponent<TransformComponent>().position = Vector3(10,11,-10);
	player.AddComponent<CameraComponent>();

	// TODO : In RenderQueue
	//	-Output buffer id ?
	//	-Textures

	// TODO : load Mesh from file
	// TODO : add pivot to mesh (Anchor point for rotations, negative translation before the object matrix)
	// TODO : use resource(asset) manager
	// TODO : implement this for faster irradiance generation : https://graphics.stanford.edu/papers/ravir_thesis/chapter4.pdf
	// TODO : make mesh.static option
	// TODO : make mesh int and short mode ? (indices)
	// TODO : Textures in render commands

	const float moveSpeed = 1.0f;
	const float rotationSpeed = 40000.0f;
	const float cubeSpeed = 90.0f;

	float roll = 0.0f, pitch = 0.0f;

	float ballDirection = 0.5f;

	RenderApi::Init();
	while (!win.ShouldClose())
	{
		Time::StartNewFrame();

		if (Inputs::GetAction("Close").IsDown())
			win.Close();

		// FPS Controls
		auto& playerTransform = player.GetComponent<TransformComponent>();
		playerTransform.position += playerTransform.Forward() * (Inputs::GetAction("MoveForward").GetValue() * moveSpeed * Time::DeltaTime());
		playerTransform.position += playerTransform.Right() * (Inputs::GetAction("MoveRight").GetValue() * moveSpeed * Time::DeltaTime());
		playerTransform.position += playerTransform.Up() * (Inputs::GetAction("MoveUp").GetValue() * moveSpeed * Time::DeltaTime());

		roll += rotationSpeed * -Inputs::GetAction("LookRight").GetValue() * Time::DeltaTime();
		pitch += rotationSpeed * -Inputs::GetAction("LookUp").GetValue() * Time::DeltaTime();

		pitch = Scalar::Clamp(pitch, -89.999f, 89.999f);

		playerTransform.rotation = Quaternion::AngleAxis(roll, Directions::Up);
		playerTransform.rotation.Rotate(pitch, playerTransform.Right());

		ForwardRenderer::RenderScene(scene, player.GetComponent<CameraComponent>());

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;


*/