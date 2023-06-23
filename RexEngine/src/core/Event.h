#pragma once

#include <entt/entt.hpp>

namespace RexEngine
{
	/* Usage:
	* 
	*	void f();
	* 
	*	struct g {
	*		void f();
	*	};
	* 
	*	Event<> ev; // Event<Argument types>
	*	g instance;
	*	ev.Register<&f>();
	*	ev.Register<&g::f>(instance);
	*
	*	ev.UnRegister<&g::f>(instance);
	*	ev.UnRegister<&f>();
	*	ev.Dispatch();
	*/
	template<typename... Args>
	class Event
	{
	public:

		Event()
			: m_sink(m_signal)
		{ }

		template<auto T>
		void Register()
		{
			m_sink.connect<T>();
		}

		template<auto T>
		void Register(auto&& instance)
		{
			m_sink.connect<T>(std::forward<decltype(instance)>(instance));
		}

		template<auto T>
		void UnRegister()
		{
			if (m_signal.size() != 0) // de-static fiasco ...
				m_sink.disconnect<T>();
		}

		template<auto T>
		void UnRegister(auto&& instance)
		{
			if(m_signal.size() != 0) // de-static fiasco ...
				m_sink.disconnect<T>(std::forward<decltype(instance)>(instance));
		}

		void Dispatch(Args... args) const
		{
			m_signal.publish(args...);
		}

	private:
		entt::sigh<void(Args...)> m_signal;
		entt::sink<decltype(m_signal)> m_sink;
	};
}