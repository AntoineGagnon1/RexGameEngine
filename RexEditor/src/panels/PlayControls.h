#pragma once

#include <RexEngine.h>

#include "Panel.h"
#include "PanelManager.h"
#include "ui/UIElements.h"

#include "../core/EditorAssets.h"
#include "SceneView.h"

namespace RexEditor
{
	class PlayControlsPanel : public Panel
	{
	public:
		PlayControlsPanel() 
			: Panel("Play Controls")
		{ }


		// Is the Editor in play mode ? (running or paused)
		bool IsPlayMode() const { return m_started; }

		// Is the Editor in play mode AND running ? (not paused)
		bool IsPlaying() const { return m_playing; }

	protected:
		virtual void OnGui([[maybe_unused]] float deltaTime) override
		{
			// Play/Pause Button
			float size = PanelSize().y - 35;
			if (UI::ImageButton playPause("PlayPause", m_playing ? EditorAssets::Pause() : EditorAssets::Play(), {size, size}); playPause.IsClicked())
			{
				if (!m_playing)
				{
					Cursor::SetCursorMode(CursorMode::Locked);
				}

				if (!m_playing && !m_started)
				{
					m_started = true;

					// Save the current scene
					auto scene = RexEngine::Scene::CurrentScene();
					RexEngine::AssetManager::SaveAsset<RexEngine::Scene>(scene.GetAssetGuid());

					// Focus on the Game View
					auto panel = PanelManager::GetPanel("Game View");
					if (panel != nullptr)
					{
						panel->Show();
						panel->SetFocused();
					}
				}

				m_playing = !m_playing;
			}

			if (m_started)
			{ // Stop button
				UI::SameLine();
				if (UI::ImageButton stop("Stop", EditorAssets::Stop(), { size, size }); stop.IsClicked())
				{
					m_playing = false;
					m_started = false;

					// Reload scene from file
					RexEngine::AssetManager::ReloadAsset<RexEngine::Scene>(RexEngine::Scene::CurrentScene().GetAssetGuid());

					// Focus the Scene View
					auto panel = PanelManager::GetPanel<SceneViewPanel>();
					if (panel != nullptr)
					{
						panel->Show();
						panel->SetFocused();
					}
				}
			}
		}

	private:
		bool m_playing = false; // Will be false if the game is paused
		bool m_started = false; // Will only be false if stop is pressed
	};
}