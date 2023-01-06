#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <RexEngine.h>

#include "Panel.h"
#include "ui/UIElements.h"

namespace RexEditor
{
	class ConsolePanel : public Panel
	{
	public:
		ConsolePanel() : Panel("Console")
		{
			RexEngine::Log::LogEvent().Register<&ConsolePanel::OnLog>(this);
		}

		~ConsolePanel()
		{
			RexEngine::Log::LogEvent().UnRegister<&ConsolePanel::OnLog>(this);
		}

	protected:
		virtual void OnGui(float deltaTime) override
		{
			if (UI::Button clear(std::format("Clear ({})", m_messages.size())); clear.IsClicked())
			{
				m_messages.clear();
			}

			SetUnsaved(m_messages.size() > 0);

			UI::Separator();

			if (UI::Table table("logMessageTable", 4, UI::TableFlags::Resizable); table.IsVisible())
			{
				for (auto& msg : m_messages)
				{
					static constexpr std::array<const char*, 5> TypeToString = { "Debug" , "Info", "Warning", "Error", "Assert" };
					static constexpr std::array<RexEngine::Color, 5> TypeToColor = { Color(0,1,0) , Color(1,1,1), Color(1,1,0), Color(1,0,0), Color(1,0.5,0) };

					// Type
					table.NextElement();
					UI::PushFontColor(TypeToColor[(int)msg.type]);
					UI::FramedText(std::string(TypeToString[(int)msg.type]));

					// Time
					table.NextElement();
					auto time_t = std::chrono::system_clock::to_time_t(msg.time);
					auto time = std::localtime(&time_t);
					auto mili = std::chrono::time_point_cast<std::chrono::milliseconds>(msg.time).time_since_epoch() % 1000;
					UI::FramedText(std::format("{}:{}:{}:{}", time->tm_hour, time->tm_min, time->tm_sec, mili));

					// Message
					table.NextElement();
					UI::Text text(msg.msg);
					UI::PopFontColor();

					// file:line(func)
					table.NextElement();
					UI::PushFontColor(Color(0.5f, 0.5f, 0.5f));
					UI::Text context(std::format("{}:{}({})", std::filesystem::path(msg.fileName).filename().string(), msg.line, msg.funcName));
					UI::PopFontColor();
				}
			}
		}

	private:

		void OnLog(RexEngine::Log::LogType type, const std::string& msg, uint_least32_t line, const std::string& func, const std::string& file)
		{
			m_messages.push_back({type, msg, line, func, file, std::chrono::system_clock::now()});
		}

		struct ConsoleMessage
		{
			RexEngine::Log::LogType type;
			std::string msg;
			uint_least32_t line;
			std::string funcName;
			std::string fileName;
			std::chrono::system_clock::time_point time;
		};

		std::vector<ConsoleMessage> m_messages;
	};
}