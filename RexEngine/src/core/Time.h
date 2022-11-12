#pragma once
#include <chrono>


namespace RexEngine
{
	class Time
	{
	public:

		// Delta time in seconds
		inline static float DeltaTime()
		{
			using namespace std::chrono_literals;
			return (float)(deltaTime / 1.0s);
			std::string test = "123123";
		}

		// Call this once at the start of every frame
		inline static void StartNewFrame()
		{
			static ClockType::time_point timeLastFrame = ClockType::now(); // The first frame will have a delta time of 0
			auto now = ClockType::now();
			
			deltaTime = now - timeLastFrame;

			timeLastFrame = now;
		}

	private:
		using ClockType = std::chrono::steady_clock;

		inline static ClockType::duration deltaTime;
	};
}