#pragma once

#include <string>
#include <RexEngine.h>

namespace RexEditor
{
	struct HideShaderAttribute
	{
		HideShaderAttribute(const std::string& _) { }
	};

	struct SliderShaderAttribute
	{
		float Min, Max;

		SliderShaderAttribute(const std::string& argsStr)
			: Min(0.0f), Max(0.0f)
		{
			auto args = RexEngine::StringHelper::Split(argsStr, ',');

			if (args.size() >= 1)
				Min = std::stof(args[0]);
			if (args.size() >= 2)
				Max = std::stof(args[1]);
		}
	};

	class ShaderAttributes
	{
		RE_STATIC_CONSTRUCTOR({
			RexEngine::Shader::RegisterAttribute<HideShaderAttribute>("Hide");
			RexEngine::Shader::RegisterAttribute<SliderShaderAttribute>("Slider");

		});
	};
}