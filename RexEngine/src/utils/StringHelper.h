#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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

	inline std::string& ReplaceChars(std::string& source, char toReplace, char replaceWith)
	{
		std::replace_if(source.begin(), source.end(),
			[&toReplace](std::string::value_type v) { return v == toReplace; },
			replaceWith);
		return source;
	}
}