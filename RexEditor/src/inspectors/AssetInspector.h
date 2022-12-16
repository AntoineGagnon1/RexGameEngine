#pragma once

#include <RexEngine.h>

#include "../ui/UIElements.h"

namespace RexEditor
{
	class AssetInspector
	{
	public:
		inline static void InspectAsset(float deltaTime, RexEngine::AssetType type, const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;

			if (type.type == typeid(Shader))
				InspectShader(assetPath);
			else
				UI::Text("Invalid Asset !");
		}

	private:

		template<typename T>
		inline static RexEngine::Asset<T> GetAsset(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto guid = AssetManager::GetAssetGuidFromPath(assetPath);

			if (guid == Guid::Empty)
			{ // Load the asset into the registry
				guid = Guid::Generate();
				if (!AssetManager::AddAsset<T>(guid, assetPath))
					RE_LOG_ERROR("Could not load the asset at {}!", assetPath.string());
			}

			return AssetManager::GetAsset<Shader>(guid);
		}

		inline static void InspectShader(const std::filesystem::path& assetPath)
		{
			auto shader = GetAsset<Shader>(assetPath);

			UI::Text("Shader");
		}

	};
}