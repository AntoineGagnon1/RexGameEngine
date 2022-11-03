#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace RexEngine::StringHelper
{
	inline std::vector<std::string> Split(const std::string& from, char delimiter)
	{
		std::vector<std::string> result;
		std::istringstream ss(from);

		std::string token;
		while (std::getline(ss, token, delimiter)) 
		{
			result.push_back(token);
		}

		return result;
	}
}