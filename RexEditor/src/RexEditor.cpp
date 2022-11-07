#include <iostream>

#include <RexEngine.h>
#include <span>

using namespace RexEngine;

void MakeSphereMesh(std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<unsigned int>& indices);

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
	shader.SetUniformVector3("albedo", Vector3(1.0f, 0.0f, 0.0f));
	shader.SetUniformFloat("metallic", 0.5f);
	shader.SetUniformFloat("roughness", 0.5f);
	shader.SetUniformFloat("ao", 1.0f);

	auto shaderPtr = std::make_shared<Shader>(shader);
	
	// TODO : load from file
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<unsigned int> indices;
	MakeSphereMesh(vertices, normals, indices);
	auto mesh = Mesh(std::span<Vector3>(vertices), std::span<unsigned int>(indices), std::span<Vector3>(normals)); // TODO : add pivot to mesh (Anchor point for rotations, negative translation before the object matrix)
	auto meshPtr = std::make_shared<Mesh>(mesh);

	Scene scene;
	auto light = scene.CreateEntity();
	auto& lightTransform = light.AddComponent<TransformComponent>();
	lightTransform.position = Vector3(0, 0, -3);
	lightTransform.scale = Vector3(0.1f, 0.1f, 0.1f);
	auto& lightMesh = light.AddComponent<MeshRendererComponent>();
	lightMesh.shader = shaderPtr;
	lightMesh.mesh = meshPtr;

	auto cube = scene.CreateEntity();
	cube.AddComponent<TransformComponent>();
	auto& meshRenderer = cube.AddComponent<MeshRendererComponent>();
	meshRenderer.cullingMode = RenderApi::CullingMode::Front;
	meshRenderer.mesh = meshPtr;
	meshRenderer.shader = shaderPtr; // TODO : use resource(asset) manager

	auto player = scene.CreateEntity();
	player.AddComponent<TransformComponent>().position.z = -2;
	player.AddComponent<CameraComponent>();

	// TODO : In RenderQueue
	//	-Output buffer id ? 
	//	-Textures
	
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

		// Rotate the cube
		//cube.GetComponent<TransformComponent>().rotation.Rotate(cubeSpeed * Time::DeltaTime(), Directions::Up);
		cube.GetComponent<TransformComponent>().position.y += ballDirection * Time::DeltaTime();

		if (abs(cube.GetComponent<TransformComponent>().position.y) > 1.0f)
			ballDirection *= -1.0f;

		RenderApi::ClearColorBit(); // TODO : move these to the forward renderer
		RenderApi::ClearDepthBit();

		ForwardRenderer::RenderScene(scene, player.GetComponent<CameraComponent>()); // TODO : Lights

		win.SwapBuffers();
		Inputs::PollInputs();
	}

	return 0;
}


void MakeSphereMesh(std::vector<Vector3>& vertices, std::vector<Vector3>& normals, std::vector<unsigned int>& indices)
{ // From : http://www.songho.ca/opengl/gl_sphere.html
	const float PI = 3.1415926535897;
	const int stackCount = 32;
	const int sectorCount = 32;
	const float radius = 1.0f;

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices.push_back(Vector3(x, y, z));

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(Vector3(nx, ny, nz));

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			//texCoords.push_back(Vector2(s,t));
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}

/*
std::vector<Vector3> vertices = {
		{-0.5f,-0.5f,-0.5f}, {-0.5f,0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f},
		{0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f}, {0.5f,-0.5f,0.5f}, {0.5f,-0.5f,-0.5f},
		{-0.5f,-0.5f,0.5f}, {-0.5f,0.5f,0.5f}, {-0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,0.5f},
		{0.5f,0.5f,0.5f}, {-0.5f,0.5f,0.5f}, {-0.5f,-0.5f,0.5f}, {-0.5f,-0.5f,0.5f}, {0.5f,-0.5f,0.5f}, {0.5f,0.5f,0.5f},
		{-0.5f,0.5f,-0.5f}, {-0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f}, {0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f},
		{-0.5f,-0.5f,0.5f}, {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,0.5f}, {-0.5f,-0.5f,0.5f}
	};

	std::vector<Vector3> normals = {
		{0,0,-1},{0,0,-1},{0,0,-1},{0,0,-1},{0,0,-1},{0,0,-1},
		{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,0,0},
		{-1,0,0},{-1,0,0},{-1,0,0},{-1,0,0},{-1,0,0},{-1,0,0},
		{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},
		{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,1,0},
		{0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0}
	};

	std::vector<unsigned int> indices = {
		0,1,2, 3,4,5, 6,7,8, 9,10,11, 12,13,14, 15,16,17, 18,19,20, 21,22,23, 24,25,26, 27,28,29, 30,31,32, 33,34,35
	};

*/