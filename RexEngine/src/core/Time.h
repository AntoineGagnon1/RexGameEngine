#pragma once
#include <chrono>


namespace RexEngine
{
	class Timer
	{
	public:

		void Start()
		{
			if (m_paused)
			{
				m_paused = false;
				m_timeLast = ClockType::now();
			}
		}

		void Pause()
		{
			if (!m_paused)
			{
				m_elapsed += ClockType::now() - m_timeLast;
				m_paused = true;
			}
		}

		void Restart()
		{
			using namespace std::chrono_literals;
			m_paused = true;
			m_elapsed = 0s;
			Start();
		}
		
		double ElapsedSeconds()
		{
			using namespace std::chrono_literals;
			if (!m_paused)
			{
				Pause(); // Update m_elapsed
				Start();
			}

			return (float)(m_elapsed / 1.0s);
		}


	private:
		using ClockType = std::chrono::steady_clock;
		
		ClockType::time_point m_timeLast;
		ClockType::duration m_elapsed;
		bool m_paused = false;
	};

	class Time
	{
	public:

		// Delta time in seconds
		inline static float DeltaTime()
		{
			using namespace std::chrono_literals;
			return (float)(deltaTime / 1.0s);
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