#pragma once

#include <string>
#include <functional>
#include "../math/Vectors.h"

struct GLFWwindow;

namespace RexEngine
{
	class Window
	{
	public:

		Window(const std::string& title, int width, int height);
		~Window();

		bool ShouldClose();
		void Close();
		void MakeActive();

		void SwapBuffers();


		void SetResizeCallback(std::function<void(Vector2Int)> callback);

		GLFWwindow* WindowHandle() { return m_window; }


		static void SetVSync(bool state);

		inline static Window* ActiveWindow() { return s_activeWindow; }
	
	private:
		static void HandleResize(GLFWwindow* window, int width, int height);

	private:
		GLFWwindow* m_window;

		std::function<void(Vector2Int)> m_resizeCallback;

		inline static Window* s_activeWindow = nullptr;
	};
}