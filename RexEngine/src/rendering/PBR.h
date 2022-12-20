#pragma once

#include <memory>
#include <string>

#include "Cubemap.h"
#include "Texture.h"
#include "../math/Vectors.h"

namespace RexEngine
{
	// Some utils functions related to the PBR workflow
	class PBR
	{
	public:
		// Sample delta : accuracy of the result
		static std::shared_ptr<Cubemap> CreateIrradianceMap(const Cubemap& cubemapFrom, int size, float sampleDelta = 0.025f);

		// Create a pre-filter map for roughness of the environment map passed as parameter
		// The map for each roughness level is stored as a mipmap
		static std::shared_ptr<Cubemap> CreatePreFilterMap(const Cubemap& cubemapFrom, int size);

		// Brdf lookup table
		static std::shared_ptr<Texture> CreateBRDFLut(Vector2Int size);
	};
}