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

			bool needsSave = false;

			// Shader
			auto shader = mat->GetShader();
			UI::AssetInput<Shader> shaderIn("Shader", shader);

			{
				UI::Anchor a(UI::AnchorPos::Center);
				if (UI::Button b("Reload Shader"); b.IsClicked() || shaderIn.HasChanged()) // Has change or reload
				{
					mat->SetShader(shader);
					needsSave = true;
				}
			}

			// Uniforms
			auto names = mat->GetUniforms();
			for (auto& name : names)
			{
				auto& uniform = mat->GetUniform(name);

				if (std::holds_alternative<float>(uniform))
				{
					UI::FloatInput in(name, std::get<float>(uniform));
					needsSave |= in.HasChanged();
				}
				else if (std::holds_alternative<Vector3>(uniform))
				{
					UI::Vector3Input in(name, std::get<Vector3>(uniform));
					needsSave |= in.HasChanged();
				}
				else if (std::holds_alternative<Matrix4>(uniform))
				{
					UI::Matrix4Input in(name, std::get<Matrix4>(uniform));
					needsSave |= in.HasChanged();
				}
				else if (std::holds_alternative<Asset<Texture>>(uniform))
				{
					// TODO : preview square
					UI::AssetInput<Texture> in(name, std::get<Asset<Texture>>(uniform));
					needsSave |= in.HasChanged();
				}
			}

			// Save the changes
			if (needsSave)
				AssetManager::SaveAsset<Material>(mat.GetAssetGuid());
		}

	};
}