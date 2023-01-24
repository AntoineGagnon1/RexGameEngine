#include "REDPch.h"

#include "../ui/UIElements.h"
#include "../ui/UI.h"

#include "../panels/Inspector.h"

#include "core/ShaderAttributes.h"

namespace RexEditor::AssetInspectors
{

	RE_STATIC_CONSTRUCTOR({

		// Shader
		InspectorPanel::AssetInspectors().Add<Shader>([] (const Guid& guid) {
			auto shader = AssetManager::GetAsset<Shader>(guid);

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
			UI::ComboBoxEnum<RenderApi::DepthFunction> depthFunc("Depth function", { "Less", "Less Equal", "Greater", "Greater Equal" }, shader->DepthFunction());

			// Changed
			if (cullingMode.HasChanged() || priority.HasChanged() || depthFunc.HasChanged())
				AssetManager::SaveAsset<Shader>(shader.GetAssetGuid());

			UI::Separator();

			// Source
			if (UI::TreeNode n("Source", UI::TreeNodeFlags::DefaultOpen | UI::TreeNodeFlags::Framed); n.IsOpen())
			{
				std::ifstream file(AssetManager::GetAssetPathFromGuid(guid));
				if (!file.is_open())
					UI::Text("Could not open the source file !");

				std::stringstream strStream;
				strStream << file.rdbuf();
				auto content = strStream.str();

				UI::Text t(content);
			}
		});

		// Material
		InspectorPanel::AssetInspectors().Add<Material>([](const Guid& guid) {
			auto mat = AssetManager::GetAsset<Material>(guid);

			bool needsSave = false;

			// Shader
			auto shader = mat->GetShader();
			UI::AssetInput<Shader> shaderIn("Shader", shader);
			if (shaderIn.HasChanged())
				mat->SetShader(shader);

			{
				UI::Anchor a(UI::AnchorPos::Right);
				if (UI::Button b("Reload Shader"); b.IsClicked() || shaderIn.HasChanged()) // Has change or reload
				{
					AssetManager::ReloadAsset<Shader>(shader.GetAssetGuid());
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

		});

		// Texture
		InspectorPanel::AssetInspectors().Add<Texture>([](const Guid& guid) {
			auto texture = AssetManager::GetAsset<Texture>(guid);

			static RenderApi::TextureTarget tempTarget = RenderApi::TextureTarget::Texture2D;
			static RenderApi::PixelFormat tempFormat = RenderApi::PixelFormat::RGB;
			static bool tempHdr = false;
			static bool tempFlipY = false;
			static RenderApi::TextureOptionValue tempWrapS = RenderApi::TextureOptionValue::Repeat;
			static RenderApi::TextureOptionValue tempWrapT = RenderApi::TextureOptionValue::Repeat;
			static Guid lastGuid = Guid::Empty;

			if (texture.GetAssetGuid() != lastGuid)
			{ // Just started inspecting this texture
				tempTarget = texture->GetTarget();
				tempFormat = texture->GetFormat();
				tempHdr = texture->GetHdr();
				tempFlipY = texture->GetFlipY();
				tempWrapS = texture->GetOption(RenderApi::TextureOption::WrapS);
				tempWrapT = texture->GetOption(RenderApi::TextureOption::WrapT);
				lastGuid = texture.GetAssetGuid();
			}

			auto tempSize = texture->Size();
			UI::ReadOnly<UI::Vector2IntInput> size("Size", tempSize);

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
			UI::ComboBoxEnum<RenderApi::TextureOptionValue> wrapS("Wrap X", { "Repeat", "Clamp to edge" }, tempWrapS);
			UI::ComboBoxEnum<RenderApi::TextureOptionValue> wrapT("Wrap Y", { "Repeat", "Clamp to edge" }, tempWrapT);


			if (UI::Button apply("Apply Changes"); apply.IsClicked())
			{
				texture->ChangeSettings(tempTarget, tempFormat, tempFlipY, tempHdr);
				AssetManager::SaveAsset<Texture>(texture.GetAssetGuid());
				AssetManager::ReloadAsset<Texture>(texture.GetAssetGuid());
			}
		});

		// CubeMap
		InspectorPanel::AssetInspectors().Add<Cubemap>([](const Guid& guid) {
			auto cubemap = AssetManager::GetAsset<Cubemap>(guid);

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
				AssetManager::AddAsset<Cubemap>(cubemap.GetAssetGuid(), AssetManager::GetAssetPathFromGuid(guid), Asset<Cubemap>(cubemap.GetAssetGuid(), newCubemap));
				AssetManager::ReloadAsset<Cubemap>(cubemap.GetAssetGuid());
			}
		
		});

		// Mesh
		InspectorPanel::AssetInspectors().Add<Mesh>([](const Guid& guid) {
			auto mesh = AssetManager::GetAsset<Mesh>(guid);

			UI::Text vertices(std::format( "Vertex Count   :   {}", mesh->GetVertexCount()));
			UI::Text triangles(std::format("Triangle Count : {}", mesh->GetIndexCount() / 3));
			UI::Text normals(std::format(  "Has normals : {}", mesh->HasNormals()));
			UI::Text uvs(std::format(	   "Has UVs     : {}", mesh->HasUVs()));
		});

	});


}