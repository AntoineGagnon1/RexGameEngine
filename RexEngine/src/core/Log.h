#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <source_location>
#include <filesystem>

#include "Event.h"


namespace RexEngine::Log
{
	enum class LogType { Debug, Info, Warning, Error, Assert };

	// Usage : LogEvent().Register<...>();
	inline auto& LogEvent() { static RexEngine::Event<LogType, // type 
																  const std::string&, // msg
																  uint_least32_t, // line
																  const std::string&, // func_name 
																  const std::string&> e; // file_name
									return e; }


	inline void DispatchLog(LogType type, const std::source_location& location, const std::string& msg)
	{
		LogEvent().Dispatch(type, msg, location.line(), std::string(location.function_name()), std::string(location.file_name()));
	}

	// Print using the std::format syntax, in debug mode only
	#ifdef RE_DEBUG
		#define RE_LOG_DEBUG(FMT, ...) RexEngine::Log::DispatchLog(RexEngine::Log::LogType::Debug, std::source_location::current(), std::format(FMT, __VA_ARGS__))
	#else
		#define RE_LOG_DEBUG(FMT, ...) 
	#endif

	// Print using the std::format syntax
	#define RE_LOG_INFO(FMT, ...) RexEngine::Log::DispatchLog(RexEngine::Log::LogType::Info, std::source_location::current(), std::format(FMT, __VA_ARGS__))

	// Print using the std::format syntax
	#define RE_LOG_WARN(FMT, ...) RexEngine::Log::DispatchLog(RexEngine::Log::LogType::Warning, std::source_location::current(), std::format(FMT, __VA_ARGS__))

	// Print using the std::format syntax
	#define RE_LOG_ERROR(FMT, ...) RexEngine::Log::DispatchLog(RexEngine::Log::LogType::Error, std::source_location::current(), std::format(FMT, __VA_ARGS__))
}


// ASSETS

#ifdef RE_WIN64
#include <intrin.h>
#define RE_DEBUG_BREAK() __debugbreak()
#endif // RE_WIN64

#include "Log.h"

// TODO : ifdef to enable asserts

// Will output the message to the console if the condition was false.
// The message can use the same format as the Logger
#define RE_ASSERT(condition, FMT, ...) { if(!(condition)){ RexEngine::Log::DispatchLog(RexEngine::Log::LogType::Assert, std::source_location::current(), std::format(FMT, __VA_ARGS__)); RE_DEBUG_BREAK(); }}
