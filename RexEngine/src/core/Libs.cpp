#include "REPch.h"
#include "Libs.h"

#include "window/Window.h"

#include "rendering/RenderApi.h"

namespace RexEngine
{
	Libs::Libs()
	{
		// GLFW
		if (!glfwInit())
			RE_LOG_ERROR("Could not init glfw !");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GlfwVersionMajor);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GlfwVersionMinor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Create a temp window to init glad
		Window win("", 1, 1);
		win.MakeActive();
		
		// GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			RE_LOG_ERROR("Could not init glad !");

		// Enable face culling
		RenderApi::Init();
	}

	Libs::~Libs()
	{
		glfwTerminate();
	}
}