#pragma once

#include <string>
#include <filesystem>

namespace RexEngine::Dirs
{
	// Location of directories at runtime, relative to the working directory

	inline const std::filesystem::path ScriptDir("Dotnet");
	inline const std::filesystem::path ScriptEngineDir(ScriptDir / "ScriptEngine");
}

namespace RexEngine::Files
{
	// Location of files at runtime, relative to the working directory

}