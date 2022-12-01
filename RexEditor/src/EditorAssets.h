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

	private:

		inline static void LoadAssets()
		{
			s_folderIcon = RexEngine::Texture::FromFile("assets/icons/folder.png");
			s_fileIcon = RexEngine::Texture::FromFile("assets/icons/file.png");
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStarted().Register<&EditorAssets::LoadAssets>();
		});

	private:
		inline static std::shared_ptr<RexEngine::Texture> s_folderIcon;
		inline static std::shared_ptr<RexEngine::Texture> s_fileIcon;

	};
}