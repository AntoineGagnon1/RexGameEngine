#pragma once

#include <vector>
#include <unordered_map>
#include <any>
#include <concepts>
#include <string>

#include "RenderApi.h"
#include "Mesh.h"
#include "Material.h"

namespace RexEngine
{
	// Current RenderQueue priorities:
	// Opaque : 0
	// Transparent : 1000
	// Gizmos (Editor) : 10000

	template<typename T>
	concept RenderCommandType =
		requires() { T{}; } // Default constructible
		&& requires(const T t1, const T t2) { {t1 < t2} -> std::convertible_to<bool>; } // Needs to be sorted
		&& requires(const T t, const T& last) { t.Render(last); };  // Do the render call, the element rendered just before is passed as parameter
																	// if this is the first element, T() will be passed

	class RenderQueue
	{
	public:

		RenderQueue() : RenderQueue(0) {}

		RenderQueue(RenderQueue&& from) noexcept
			: m_priority(from.m_priority),
			m_queue(std::move(from.m_queue)),
			m_render(std::move(from.m_render)),
			m_sort(std::move(from.m_sort)),
			m_clear(std::move(from.m_clear))
		{ }

		RenderQueue(const RenderQueue&) = delete;
		RenderQueue& operator=(const RenderQueue&) = delete;

		template<RenderCommandType T>
		static RenderQueue MakeRenderQueue(int priority)
		{
			RenderQueue queue(priority);
			queue.m_queue = std::vector<T>();
			queue.m_render = std::bind(RenderQueue::RenderTemplate<T>, std::placeholders::_1);
			queue.m_sort = std::bind(RenderQueue::SortTemplate<T>, std::placeholders::_1);
			queue.m_clear = std::bind(RenderQueue::ClearTemplate<T>, std::placeholders::_1);

			return queue;
		}

		void Sort()
		{
			m_sort(m_queue);
		}

		// Should probably be sorted using Sort() first !
		void Render() const
		{
			m_render(m_queue);
		}

		void Clear()
		{
			m_clear(m_queue);
		}

		auto GetPriority() const { return m_priority; }

		template<RenderCommandType T, typename ...Args>
		void AddCommand(Args&&... args)
		{
			GetData<T>().emplace_back(std::forward<Args>(args)...);
		}

		template<RenderCommandType T>
		std::vector<T>& GetData()
		{
			return std::any_cast<std::vector<T>&>(m_queue);
		}

		template<RenderCommandType T>
		const std::vector<T>& GetData() const
		{
			return std::any_cast<const std::vector<T>&>(m_queue);
		}

		friend bool operator<(const RenderQueue& lhs, const RenderQueue& rhs) { return lhs.m_priority < rhs.m_priority; }

	private:

		RenderQueue(int priority)
			: m_priority(priority)
		{ }

		template<RenderCommandType T>
		static void RenderTemplate(const std::any& data)
		{
			auto& vec = std::any_cast<const std::vector<T>&>(data);
			T first = T();
			T& last = first;
			for (auto& element : vec)
			{
				element.Render(last);
				last = element;
			}
		}

		template<RenderCommandType T>
		static void SortTemplate(std::any& data)
		{
			auto& vec = std::any_cast<std::vector<T>&>(data);
			std::sort(vec.begin(), vec.end());
		}

		template<RenderCommandType T>
		static void ClearTemplate(std::any& data)
		{
			auto& vec = std::any_cast<std::vector<T>&>(data);
			vec.clear();
		}

	private:
		int m_priority;
		std::any m_queue;

		std::function<void(const std::any&)> m_render;
		std::function<void(std::any&)> m_sort;
		std::function<void(std::any&)> m_clear;
	};

	class RenderQueues
	{
	public:
		// Split add and get to prevent typos
		// a priority of -1 will be rendered before a priority of 1
		template<RenderCommandType T>
		static void AddQueue(const std::string& name, int priority)
		{
			auto& queues = GetQueueMap();
			
			RE_ASSERT(!queues.contains(name), "Already a RenderQueue named {}", name);
			queues.emplace(name, RenderQueue::MakeRenderQueue<T>(priority));
		}

		template<RenderCommandType T>
		static RenderQueue& GetQueue(const std::string& name)
		{
			auto& queues = GetQueueMap();

			RE_ASSERT(queues.contains(name), "No RenderQueue named {}", name);
			return queues[name];
		}

		static void ExecuteQueues()
		{
			// Sort by priority
			std::vector<RenderQueue*> queues;
			for (auto& queue : GetQueueMap())
				queues.push_back(&queue.second);

			std::sort(queues.begin(), queues.end(), [](RenderQueue* a, RenderQueue* b) { return (*a) < (*b); });

			// Sort and Render
			for (auto& queue : queues)
			{
				queue->Sort();
				queue->Render();
			}
		}

		static void ClearQueues()
		{
			for (auto& queue : GetQueueMap())
				queue.second.Clear();
		}

	private:

		static std::unordered_map<std::string, RenderQueue>& GetQueueMap()
		{
			static std::unordered_map<std::string, RenderQueue> queues;
			return queues;
		}

	};
}