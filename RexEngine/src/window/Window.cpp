#include "REPch.h"
#include "Window.h"

#include <core/Libs.h>
#include <stb/stb_image.h>

namespace RexEngine
{

	Window::Window(const std::string& title, int width, int height, int msaa)
	{
		glfwWindowHint(GLFW_SAMPLES, msaa);

		m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		if (!m_window)
		{
			RE_LOG_ERROR("Window creation failed !");
			return;
		}

		// Set the user data ptr to this, so callbacks can use this instance
		glfwSetWindowUserPointer(m_window, this);

		// Set the resize callback
		glfwSetFramebufferSizeCallback(m_window, &Window::HandleResize);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);

		if (s_activeWindow == this)
			s_activeWindow = nullptr;
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}

	void Window::Close()
	{
		glfwSetWindowShouldClose(m_window, true);
	}

	void Window::MakeActive()
	{
		s_activeWindow = this;
		glfwMakeContextCurrent(m_window);
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

	void Window::SetResizeCallback(std::function<void(Vector2Int)> callback)
	{
		m_resizeCallback = callback;
	}

	Vector2Int Window::GetSize() const
	{
		Vector2Int size;
		glfwGetFramebufferSize(m_window, &size.x, &size.y);
		return size;
	}

	void Window::SetWindowIcon(const std::filesystem::path& icon)
	{
		GLFWimage image[1];
		image[0].pixels = stbi_load(icon.string().c_str(), &image[0].width, &image[0].height, 0, 4);
		glfwSetWindowIcon(m_window, 1, image);
		stbi_image_free(image[0].pixels);
	}

	void Window::SetVSync(bool state)
	{
		glfwSwapInterval(state ? 1 : 0);
	}

	// Private
	void Window::HandleResize(GLFWwindow* window, int width, int height)
	{
		Window* win = (Window*)glfwGetWindowUserPointer(window);

		if (win->m_resizeCallback)
			win->m_resizeCallback(Vector2Int{width, height});
	}
}
