#pragma once

#include <RexEngine.h>
#include <memory>

#include "EditorEvents.h"

namespace RexEditor
{
	class EditorAssets
	{
	public:

		inline static const RexEngine::Texture& FolderIcon() { return *s_folderIcon; }
		inline static const RexEngine::Texture& FileIcon() { return *s_fileIcon; }
		inline static const RexEngine::Texture& NoTexture() { return *s_noTexture; }
		inline static const RexEngine::Texture& Play() { return *s_play; }
		inline static const RexEngine::Texture& Pause() { return *s_pause; }
		inline static const RexEngine::Texture& Stop() { return *s_stop; }

	private:

		inline static void LoadAssets()
		{
			s_folderIcon = RexEngine::Texture::FromFile("assets/icons/folder.png");
			s_fileIcon = RexEngine::Texture::FromFile("assets/icons/file.png");
			s_noTexture = RexEngine::Texture::FromFile("assets/icons/noTexture.png");
			s_play = RexEngine::Texture::FromFile("assets/icons/play.png");
			s_pause = RexEngine::Texture::FromFile("assets/icons/pause.png");
			s_stop = RexEngine::Texture::FromFile("assets/icons/stop.png");
		}

		inline static void UnLoadAssets()
		{
			s_folderIcon = nullptr;
			s_fileIcon = nullptr;
			s_noTexture = nullptr;
			s_play = nullptr;
			s_pause = nullptr;
			s_stop = nullptr;
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStarted().Register<&EditorAssets::LoadAssets>();
			EditorEvents::OnEditorStop().Register<&EditorAssets::UnLoadAssets>();
		});

	private:
		inline static std::shared_ptr<RexEngine::Texture> s_folderIcon;
		inline static std::shared_ptr<RexEngine::Texture> s_fileIcon;
		inline static std::shared_ptr<RexEngine::Texture> s_noTexture;
		inline static std::shared_ptr<RexEngine::Texture> s_play;
		inline static std::shared_ptr<RexEngine::Texture> s_pause;
		inline static std::shared_ptr<RexEngine::Texture> s_stop;

	};
}