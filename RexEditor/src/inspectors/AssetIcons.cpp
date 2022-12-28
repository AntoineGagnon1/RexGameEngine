#include "REDPch.h"
#include "panels/FileExplorer.h"

namespace RexEditor::AssetIcons
{
	std::map<std::pair<Guid, Guid>, std::shared_ptr<Texture>> s_previews;

	const Texture& GetPreview(Asset<Material> mat, Asset<Mesh> mesh)
	{
		constexpr Vector2Int PreviewSize = {128,128};

		auto key = std::make_pair(mat.GetAssetGuid(), mesh.GetAssetGuid());
		auto result = s_previews.find(key);

		// Check if the result was cached
		if (result != s_previews.end())
			return *(result->second);
		
		// Generate the texture
		static auto scene = Asset<Scene>(Guid::Generate(), Scene::CreateScene());
		static auto camEntity = scene->CreateEntity();
		static auto& cam = camEntity.AddComponent<CameraComponent>(70.0f, 0.01f, 10.0f);
		static auto sphere = scene->CreateEntity();
		static auto& sphereRenderer = sphere.AddComponent<MeshRendererComponent>();
		sphere.Transform().position.z = 2;
		sphereRenderer.material = mat;
		sphereRenderer.mesh = mesh;

		auto texture = std::make_shared<Texture>(RenderApi::PixelFormat::RGBA, PreviewSize);

		static NoDestroy<FrameBuffer> frameBuffer;
		static NoDestroy<RenderBuffer> depth(RenderApi::PixelType::Depth, PreviewSize);

		frameBuffer->BindTexture(*texture, RenderApi::FrameBufferTextureType::Color);
		frameBuffer->BindRenderBuffer(*depth, RenderApi::FrameBufferTextureType::Depth);

		frameBuffer->Bind();
		RenderApi::SetViewportSize(PreviewSize);

		RenderApi::ClearColorBit();
		RenderApi::ClearDepthBit();

		ForwardRenderer::RenderScene(scene, cam);

		s_previews[key] = texture;
		FrameBuffer::UnBind();

		return *texture;
	}


	void OnClose()
	{
		s_previews.clear();
	}


	RE_STATIC_CONSTRUCTOR({
		// Texture
		FileExplorerPanel::RegisterIcon<Texture>([](Guid guid) -> const Texture& {
			std::shared_ptr<Texture> tex = AssetManager::GetAsset<Texture>(guid);
			return *tex;
		});


		// Material
		FileExplorerPanel::RegisterIcon<Material>([](Guid guid) -> const Texture& {
			auto material = AssetManager::GetAsset<Material>(guid);
			static NoDestroy<Asset<Mesh>> sphereMesh(Guid::Generate(), Shapes::GetSphereMesh());
			return GetPreview(material, *sphereMesh.GetPtr());
		});

		EditorEvents::OnEditorStop().Register<&AssetIcons::OnClose>();
	});
}