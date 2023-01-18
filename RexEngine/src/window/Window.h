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

		Window(const std::string& title, int width, int height, int msaa);
		~Window();

		bool ShouldClose() const;
		void Close();
		void MakeActive();

		void SwapBuffers();


		void SetResizeCallback(std::function<void(Vector2Int)> callback);

		// The size of the window, in pixels
		Vector2Int GetSize() const;

		GLFWwindow* WindowHandle() const { return m_window; }

		void SetWindowIcon(const std::filesystem::path& icon);

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