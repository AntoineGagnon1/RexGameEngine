#pragma once

#include <RexEngine.h>

namespace RexEditor::AssetIcons
{
	const Texture& GetPreview(Asset<Material> mat, Asset<Mesh> mesh);

	Asset<Material> PreviewMaterial();
}