#pragma once
#include <vector>
#include <filesystem>
#include <fstream>
#include "../core/Log.h"

namespace RexEngine::FileHelper
{
	template<typename T> requires(sizeof(T) == 1)
	inline std::vector<T> ReadAllBytes(const std::filesystem::path& path)
	{
		std::ifstream stream(path.c_str(), std::ios::binary | std::ios::ate);
		if (!stream)
		{
			RE_LOG_WARN("Could not open file at {}", path.string());
			return std::vector<T>();
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		size_t size = end - stream.tellg();

		if (size == 0)
			return std::vector<T>();

		std::vector<T> buffer(size);
		stream.read((char*)buffer.data(), size);
		stream.close();
		return buffer;
	}
}