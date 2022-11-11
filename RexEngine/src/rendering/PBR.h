#pragma once

#include <memory>
#include <string>

#include "Cubemap.h"
#include "../math/Vectors.h"

namespace RexEngine
{
	class PBR
	{
	public:

		// Project a HDRI file on a cubemap, WARNING : requires a valid opengl context
		// Size : size of each face of the final cubemap
		static std::shared_ptr<Cubemap> FromHDRI(const std::string& path, Vector2Int size);

		// Sample delta : accuracy of the result
		static std::shared_ptr<Cubemap> CreateIrradianceMap(const Cubemap& from, Vector2Int size, float sampleDelta = 0.025f);
	};
}