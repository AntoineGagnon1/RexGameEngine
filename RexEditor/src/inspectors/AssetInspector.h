#pragma once

#include <RexEngine.h>

#include "../ui/UIElements.h"
#include "../ui/UI.h"

#include "core/ShaderAttributes.h"

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
			else if (type.type == typeid(Texture))
				InspectTexture(assetPath);
			else if (type.type == typeid(Cubemap))
				InspectCubemap(assetPath);
			else if (type.type == typeid(Mesh))
				InspectMesh(assetPath);
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

		inline static void AssetHeader(const std::filesystem::path& assetPath)
		{
			{
				UI::Anchor a(UI::AnchorPos::Center);
				UI::PushFontScale(UI::FontScale::Large);
				UI::Text title(assetPath.filename().string());
				UI::Separator();
				UI::PopFontScale();
			}

			UI::EmptyLine(); // Not in the centered anchor
		}

		inline static void InspectShader(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto shader = GetAsset<Shader>(assetPath);

			AssetHeader(assetPath);

			// Valid or not
			{
				UI::Anchor a(UI::AnchorPos::Center);
				UI::PushFontColor(shader->IsValid() ? Color(0, 1, 0) : Color(1, 0, 0));
				UI::FramedText("Compilation : " + std::string((shader->IsValid() ? "Success" : "Failed")));
				UI::PopFontColor();
			}

			UI::Separator();
			UI::EmptyLine();

			// Culling mode and priority
			UI::ComboBoxEnum<RenderApi::CullingMode> cullingMode("Render faces", { "Front", "Back", "Both" }, shader->CullingMode());
			UI::ByteInput priority("Priority", shader->Priority());

			// Changed
			if (cullingMode.HasChanged() || priority.HasChanged())
				AssetManager::SaveAsset<Shader>(shader.GetAssetGuid());

			UI::Separator();
			
			// Source
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

			AssetHeader(assetPath);

			bool needsSave = false;

			// Shader
			auto shader = mat->GetShader();
			UI::AssetInput<Shader> shaderIn("Shader", shader);

			{
				UI::Anchor a(UI::AnchorPos::Right);
				if (UI::Button b("Reload Shader"); b.IsClicked() || shaderIn.HasChanged()) // Has change or reload
				{
					AssetManager::ReloadAsset<Shader>(shader.GetAssetGuid());
					mat->SetShader(AssetManager::GetAsset<Shader>(shader.GetAssetGuid()));
					needsSave = true;
				}
			}

			UI::Separator();
			UI::EmptyLine();

			// Uniforms
			auto names = mat->GetUniforms();
			for (auto& name : names)
			{
				auto attributes = mat->GetUniformAttributes(name);
				if (attributes.contains("Hide"))
					continue;

				auto& uniform = mat->GetUniform(name);

				if (std::holds_alternative<float>(uniform))
				{
					if (attributes.contains("Slider"))
					{
						auto slider = std::any_cast<SliderShaderAttribute>(attributes["Slider"]);
						UI::FloatSlider in(name, slider.Min, slider.Max, -1, std::get<float>(uniform));
						needsSave |= in.HasChanged();
					}
					else
					{ 
						UI::FloatInput in(name, std::get<float>(uniform));
						needsSave |= in.HasChanged();
					}
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
				else if (std::holds_alternative<Asset<Cubemap>>(uniform))
				{
					// TODO : preview
					UI::AssetInput<Cubemap> in(name, std::get<Asset<Cubemap>>(uniform));
					needsSave |= in.HasChanged();
				}
			}

			// Save the changes
			if (needsSave)
				AssetManager::SaveAsset<Material>(mat.GetAssetGuid());
		}


		inline static void InspectTexture(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto texture = GetAsset<Texture>(assetPath);

			static RenderApi::TextureTarget tempTarget = RenderApi::TextureTarget::Texture2D;
			static RenderApi::PixelFormat tempFormat = RenderApi::PixelFormat::RGB;
			static bool tempHdr = false;
			static bool tempFlipY = false;
			static RenderApi::TextureOptionValue tempWrapS = RenderApi::TextureOptionValue::Repeat;
			static RenderApi::TextureOptionValue tempWrapT = RenderApi::TextureOptionValue::Repeat;
			static Guid lastGuid = Guid::Empty;

			if(texture.GetAssetGuid() != lastGuid)
			{ // Just started inspecting this texture
				tempTarget = texture->GetTarget();
				tempFormat = texture->GetFormat();
				tempHdr = texture->GetHdr();
				tempFlipY = texture->GetFlipY();
				tempWrapS = texture->GetOption(RenderApi::TextureOption::WrapS);
				tempWrapT = texture->GetOption(RenderApi::TextureOption::WrapT);
				lastGuid = texture.GetAssetGuid();
			}

			AssetHeader(assetPath);

			UI::Text size(std::format("Size : ({},{})", texture->Width(), texture->Height()));
		
			UI::ComboBoxEnum<RenderApi::TextureTarget> target("Texture type", { "Texture2D" }, tempTarget);

			// Only some options are allowed based on the value of hdr
			if (tempHdr)
			{
				int selected = 0;
				tempFormat = RenderApi::PixelFormat::RGB16F;
				UI::ComboBox format("Pixel format", { "RGB16F" }, selected);
			}
			else
			{
				static constexpr RenderApi::PixelFormat SelectionToEnum[4] = {
					RenderApi::PixelFormat::Depth,
					RenderApi::PixelFormat::RG,
					RenderApi::PixelFormat::RGB,
					RenderApi::PixelFormat::RGBA
				};

				static constexpr int EnumToSelection[5] = {
					2, 3, 0,
					2, // Convert RGB16F to RGB
					1
				};

				int selected = EnumToSelection[(int)tempFormat];
				UI::ComboBox format("Pixel format", { "Depth", "RG", "RGB", "RGBA" }, selected);
				tempFormat = (RenderApi::PixelFormat)SelectionToEnum[selected];
			}
			
			UI::CheckBox flipY("Flip Y", tempFlipY);
			UI::CheckBox   hdr("Hdr   ", tempHdr); 

			UI::EmptyLine();
			// Texture options :
			UI::ComboBoxEnum<RenderApi::TextureOptionValue> wrapS("Wrap X", {"Repeat", "Clamp to edge"}, tempWrapS);
			UI::ComboBoxEnum<RenderApi::TextureOptionValue> wrapT("Wrap Y", {"Repeat", "Clamp to edge"}, tempWrapT);


			if (UI::Button apply("Apply Changes"); apply.IsClicked())
			{
				texture->ChangeSettings(tempTarget, tempFormat, tempFlipY, tempHdr);
				AssetManager::SaveAsset<Texture>(texture.GetAssetGuid());
				AssetManager::ReloadAsset<Texture>(texture.GetAssetGuid());
			}
		}

		inline static void InspectCubemap(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto cubemap = GetAsset<Cubemap>(assetPath);

			AssetHeader(assetPath);

			static RexEngine::NoDestroy<Asset<Texture>> tempSource;
			static int tempSize = 128;
			static Cubemap::ProjectionMode tempMode = Cubemap::ProjectionMode::HDRI;
			static Guid lastGuid = Guid::Empty;

			if (cubemap.GetAssetGuid() != lastGuid)
			{ // Just started inspecting this cubemap
				tempSource = cubemap->GetSource();
				tempSize = cubemap->GetSize();
				tempMode = cubemap->GetMode();
				lastGuid = cubemap.GetAssetGuid();
			}

			UI::AssetInput<Texture> source("Source Texture", *tempSource.GetPtr());
			UI::IntInput("Size", tempSize);
			UI::ComboBoxEnum<Cubemap::ProjectionMode> target("Projection Type", { "HDRI" }, tempMode);

			if (UI::Button apply("Apply Changes"); apply.IsClicked())
			{
				auto newCubemap = std::make_shared<Cubemap>(*tempSource.GetPtr(), tempSize, tempMode);
				// Overwrite the asset
				AssetManager::AddAsset<Cubemap>(cubemap.GetAssetGuid(), assetPath, Asset<Cubemap>(cubemap.GetAssetGuid(), newCubemap));
				AssetManager::ReloadAsset<Cubemap>(cubemap.GetAssetGuid());
			}
		}

		inline static void InspectMesh(const std::filesystem::path& assetPath)
		{
			using namespace RexEngine;
			auto mesh = GetAsset<Mesh>(assetPath);

			AssetHeader(assetPath);

			UI::Text vertices (std::format("Vertex Count :   {}", mesh->GetVertexCount()));
			UI::Text triangles(std::format("Triangle Count : {}", mesh->GetIndexCount() / 3));
		}
	};
}