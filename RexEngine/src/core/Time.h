#pragma once
#include <chrono>


namespace RexEngine
{
	class Time
	{
	public:

		// Delta time in seconds
		inline static double DeltaTime()
		{
			using std::chrono_literals::operator""s;
			return deltaTime / 1.0s;
		}

		// Call this once at the start of every frame
		inline static void StartNewFrame()
		{
			auto now = ClockType::now();
			
			deltaTime = now - timeLastFrame;

			timeLastFrame = now;
		}

	private:
		using ClockType = std::chrono::steady_clock;

		inline static ClockType::time_point timeLastFrame;
		inline static ClockType::duration deltaTime;
	};
}