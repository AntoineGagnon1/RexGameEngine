#pragma once

#include <RexEngine.h>

#include "../ui/UIElements.h"
#include "../ui/UI.h"

namespace RexEditor
{
	class AssetInspector
	{
	public:
		// We use assetPath instead of a guid because the asset might not be loaded yet and the FileExplorer doesnt know the type
		inline static void InspectAsset(float deltaTime, RexEngine::AssetType type, const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;

			if (type.type == typeid(Shader))
				InspectShader(assetPath);
			else if (type.type == typeid(Material))
				InspectMaterial(assetPath);
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

			return AssetManager::GetAsset<T>(guid);
		}

		inline static void InspectShader(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto shader = GetAsset<Shader>(assetPath);

			// Valid or not
			{
				UI::Anchor a(UI::AnchorPos::Center);
				UI::PushFontColor(shader->IsValid() ? Color(0, 1, 0) : Color(1, 0, 0));
				UI::FramedText("Compilation : " + std::string((shader->IsValid() ? "Success" : "Failed")));
				UI::PopFontColor();
			}

			UI::EmptyLine();

			if (UI::TreeNode n("Source", UI::TreeNodeFlags::DefaultOpen | UI::TreeNodeFlags::Framed); n.IsOpen())
			{
				std::ifstream file(assetPath);
				if (!file.is_open())
					UI::Text("Could not open the source file !");

				std::stringstream strStream;
				strStream << file.rdbuf();
				auto content = strStream.str();

				UI::Text t(content);
			}
		}

		inline static void InspectMaterial(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto mat = GetAsset<Material>(assetPath);

			UI::Text("Hello");
		}

	};
}