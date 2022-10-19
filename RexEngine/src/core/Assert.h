#pragma once

#ifdef RE_WIN64
	#include <intrin.h>
	#define RE_DEBUG_BREAK() __debugbreak()
#endif // RE_WIN64

#include "Log.h"

// TODO : ifdef to enable asserts

// Will output the message to the console if the condition was false.
// The message can use the same format as the Logger
#define RE_ASSERT(condition, ...) { if(!(condition)){ RexEngine::Log::Print(fg(fmt::color::orange_red) | fmt::emphasis::bold, "ASSERT", std::source_location::current(), __VA_ARGS__); RE_DEBUG_BREAK(); }}