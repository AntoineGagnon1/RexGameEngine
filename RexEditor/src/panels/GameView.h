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
			m_viewDepth(RexEngine::RenderApi::PixelType::Depth, { 0,0 }),
			m_stats(false), m_lastStatUpdate(0.0f), m_lastDeltaTime(0.0f), m_lastRenderTime(0.0f)
		{
			m_viewBuffer.BindTexture(m_viewTexture, RexEngine::RenderApi::FrameBufferTextureType::Color);
			m_viewBuffer.BindRenderBuffer(m_viewDepth, RexEngine::RenderApi::FrameBufferTextureType::Depth);

			m_escape = std::make_unique<RexEngine::KeyboardInput>(RexEngine::KeyCode::Escape);
			m_capture = std::make_unique<RexEngine::MouseButtonInput>(RexEngine::MouseButton::Left);
		}

	protected:
		virtual void OnResize([[maybe_unused]] RexEngine::Vector2 oldSize, RexEngine::Vector2 newSize) override
		{
			m_viewTexture.SetData((RexEngine::Vector2Int)newSize, nullptr, RexEngine::RenderApi::PixelFormat::RGB, RexEngine::RenderApi::PixelType::UByte);
			m_viewDepth.SetSize((RexEngine::Vector2Int)newSize);
		}


		virtual void OnGui([[maybe_unused]] float deltaTime) override
		{
			using namespace RexEngine;

			m_escape->PollInputs();
			m_capture->PollInputs();

			bool playing = PanelManager::GetPanel<PlayControlsPanel>()->IsPlayMode();

			if(m_escape->IsDown() && playing)
				RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Free);

			if(IsHovered() && m_capture->IsJustDown() && playing)
				RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Locked);

			// Render the current scene using the first camera found
			auto scene = Scene::CurrentScene();

			if (!scene)
				return; // No scene active, exit

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
			Timer renderTimer;
			renderTimer.Start();
			RexEngine::ForwardRenderer::RenderScene(Scene::CurrentScene(), cameras[0].second);
			renderTimer.Pause();

			// Display the texture to the ui
			Window()->DrawFullWindowTexture(m_viewTexture);

			// Revert back to the cached states
			m_viewBuffer.UnBind();
			RexEngine::RenderApi::SetViewportSize(oldViewportSize);


			// Context menu
			if (UI::ContextMenu context("GameViewContext"); context.IsOpen())
			{
				UI::CheckBox statsToggle("Show Stats", m_stats);
			}

			// Stats
			if (m_stats)
			{
				if (UI::Window stats("Stats", nullptr, UI::WindowSetting::NoTitleBar); stats.IsVisible())
				{
					if (Time::CurrentTime() >= m_lastStatUpdate + StatUpdateDelta)
					{
						m_lastStatUpdate = Time::CurrentTime();
						m_lastDeltaTime = Time::DeltaTime();
						m_lastRenderTime = renderTimer.ElapsedSeconds();
					}

					UI::Text(std::format("Fps         : {:.0f}", 1.0f / m_lastDeltaTime));
					UI::Text(std::format("Frame Time  : {:.1f}ms", m_lastDeltaTime * 1000.0f));
					UI::Text(std::format("Render Time : {:.1f}ms", m_lastRenderTime * 1000.0));
				}
			}

		}

	private:

		RexEngine::FrameBuffer m_viewBuffer;
		RexEngine::Texture m_viewTexture;
		RexEngine::RenderBuffer m_viewDepth;

		std::unique_ptr<RexEngine::Input> m_escape;
		std::unique_ptr<RexEngine::Input> m_capture;

		bool m_stats; // Show stats ?
		double m_lastStatUpdate;
		float m_lastDeltaTime;
		double m_lastRenderTime;

		static constexpr double StatUpdateDelta = 0.15f;
	};
}