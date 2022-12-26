#include "REDPch.h"
#include "panels/FileExplorer.h"

namespace RexEditor::AssetIcons
{
	RE_STATIC_CONSTRUCTOR({
		FileExplorerPanel::RegisterIcon<Texture>([](Guid guid) -> const Texture& {
			std::shared_ptr<Texture> tex = AssetManager::GetAsset<Texture>(guid);
			return *tex;
		});
	});
}