#pragma once

#pragma warning(push, 0)
#pragma warning(disable : 26495 26498 26812 26437 26451 26450 6285)
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/format.h>
#pragma warning(pop)

#include <string>
#include <string_view>
#include <utility>
#include <source_location>
#include <filesystem>


namespace RexEngine::Log
{
	// Internal use
	template <typename... Args>
	inline void Print(const fmt::v9::text_style& style, const std::string& type, const std::source_location& location, fmt::format_string<Args...>&& fmt_str, Args&&... args)
	{
		fmt::print(style, "[{}][{}({}:{})] - {}\n", type, std::filesystem::path(location.file_name()).filename().string(), location.line(), location.column(),
			fmt::format(std::forward<fmt::format_string<Args...>>(fmt_str), std::forward<Args>(args)...));
	}


	// Client Logging
	// Print using the fmt syntax, in debug mode only
	#ifdef RE_DEBUG
		#define RE_LOG_DEBUG(...) RexEngine::Log::Print(fg(fmt::color::green), "DEBUG", std::source_location::current(), __VA_ARGS__);
	#else
		#define RE_LOG_DEBUG(...)
	#endif

	// Print using the fmt syntax
	#define RE_LOG_INFO(...) RexEngine::Log::Print(fg(fmt::color::gray), "INFO", std::source_location::current(), __VA_ARGS__);

	// Print using the fmt syntax
	#define RE_LOG_WARN(...) RexEngine::Log::Print(fg(fmt::color::yellow), "WARNING", std::source_location::current(), __VA_ARGS__);

	// Print using the fmt syntax
	#define RE_LOG_ERROR(...) RexEngine::Log::Print(fg(fmt::color::red) | fmt::emphasis::bold, "ERROR", std::source_location::current(), __VA_ARGS__);
}
