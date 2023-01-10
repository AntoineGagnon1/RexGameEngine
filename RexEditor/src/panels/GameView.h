#pragma once

#include <RexEngine.h>

#include "Panel.h"
#include "ui/UIElements.h"
#include "PlayControls.h"

namespace RexEditor
{
	class GameViewPanel : public Panel
	{
	public:
		GameViewPanel() : Panel("Game View"),
			m_viewTexture(RexEngine::RenderApi::PixelFormat::RGB, { 0,0 }),
			m_viewDepth(RexEngine::RenderApi::PixelType::Depth, { 0,0 })
		{
			m_viewBuffer.BindTexture(m_viewTexture, RexEngine::RenderApi::FrameBufferTextureType::Color);
			m_viewBuffer.BindRenderBuffer(m_viewDepth, RexEngine::RenderApi::FrameBufferTextureType::Depth);

			m_escape = std::make_unique<KeyboardInput>(RexEngine::KeyCode::Escape);
			m_capture = std::make_unique<MouseButtonInput>(RexEngine::MouseButton::Left);
		}

	protected:
		virtual void OnResize(RexEngine::Vector2 oldSize, RexEngine::Vector2 newSize) override
		{
			m_viewTexture.SetData((Vector2Int)newSize, nullptr, RexEngine::RenderApi::PixelFormat::RGB, RexEngine::RenderApi::PixelType::UByte);
			m_viewDepth.SetSize((Vector2Int)newSize);
		}


		virtual void OnGui(float deltaTime) override
		{
			m_escape->PollInputs();
			m_capture->PollInputs();

			bool playing = PanelManager::GetPanel<PlayControlsPanel>()->IsPlayMode();

			if(m_escape->IsDown() && playing)
				RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Free);

			if(IsHovered() && m_capture->IsJustDown() && playing)
				RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Locked);

			// Render the current scene using the first camera found
			auto scene = Scene::CurrentScene();

			auto&& cameras = scene->GetComponents<CameraComponent>();
			if (cameras.size() <= 0)
			{
				RE_LOG_WARN("No camera in the scene !");
				return;
			}

			m_viewBuffer.Bind();

			auto oldViewportSize = RexEngine::RenderApi::GetViewportSize(); // Cache the size to revert at the end
			RexEngine::RenderApi::SetViewportSize((Vector2Int)PanelSize());

			RexEngine::RenderApi::ClearColorBit();
			RexEngine::RenderApi::ClearDepthBit();

			// Render the scene from the pov of the editor camera
			RexEngine::ForwardRenderer::RenderScene(Scene::CurrentScene(), cameras[0].second);

			// Display the texture to the ui
			Window()->DrawFullWindowTexture(m_viewTexture);

			// Revert back to the cached states
			m_viewBuffer.UnBind();
			RexEngine::RenderApi::SetViewportSize(oldViewportSize);
		}

	private:

		RexEngine::FrameBuffer m_viewBuffer;
		RexEngine::Texture m_viewTexture;
		RexEngine::RenderBuffer m_viewDepth;

		std::unique_ptr<RexEngine::Input> m_escape;
		std::unique_ptr<RexEngine::Input> m_capture;
	};
}