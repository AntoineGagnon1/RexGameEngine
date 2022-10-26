#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace RexEngine
{
	struct Libs
	{
		inline static int GlfwVersionMajor = 4; // TODO : Enable the client to change this
		inline static int GlfwVersionMinor = 6;

		Libs();
		~Libs();
	};

	inline RexEngine::Libs libs; // Will call the constructor
}