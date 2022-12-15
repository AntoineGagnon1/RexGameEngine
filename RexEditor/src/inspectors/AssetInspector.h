#pragma once

#include <RexEngine.h>

#include "../ui/UIElements.h"

namespace RexEditor
{
	class AssetInspector
	{
	public:
		inline static void InspectAsset(float deltaTime, RexEngine::AssetType type, RexEngine::Guid asset)
		{
			using namespace RexEngine;
			bool valid = false;

			if (type.type == typeid(Shader))
				valid = InspectShader(asset);

			if (!valid)
			{
				UI::Text("Invalid Asset !");
			}
		}

	private:

		inline static bool InspectShader(RexEngine::Guid guid)
		{
			using namespace RexEngine;
			auto asset = AssetManager::GetAsset<Shader>(guid);

			if (!asset)
				return false;

			UI::Text("Shader");

			return true;
		}

	};
}