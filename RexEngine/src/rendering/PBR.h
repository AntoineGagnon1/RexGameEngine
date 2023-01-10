#pragma once

#include <memory>
#include <string>

#include "Cubemap.h"
#include "Texture.h"
#include "Material.h"
#include "../math/Vectors.h"

namespace RexEngine
{

class Window;
inline Window* testWin;
	// Some utils functions related to the PBR workflow
	class PBR
	{
	public:

		inline static int IrradianceSize = 32;
		inline static float IrradianceSampleDelta = 0.025f;
		inline static int PrefilterSize = 128;
		inline static int BrdfLutSize = 512;

	private:
		// Data for the pbr shaders
		inline static int s_irradianceMapSlot;
		inline static int s_prefilterMapSlot;
		inline static int s_brdfLUTSlot;

		inline static std::shared_ptr<Cubemap> s_irradianceMap;
		inline static std::shared_ptr<Cubemap> s_prefilterMap;
		inline static std::shared_ptr<Texture> s_brdfLUT;

		// The current skybox material
		inline static Asset<Material> s_skyboxMat;

		// Sample delta : accuracy of the result
		static std::shared_ptr<Cubemap> CreateIrradianceMap(const Cubemap& cubemapFrom, int size, float sampleDelta = 0.025f);

		// Create a pre-filter map for roughness of the environment map passed as parameter
		// The map for each roughness level is stored as a mipmap
		static std::shared_ptr<Cubemap> CreatePreFilterMap(const Cubemap& cubemapFrom, int size);

		// Brdf lookup table
		static std::shared_ptr<Texture> CreateBRDFLut(Vector2Int size);

		// Reserve the texture slots
		static void Init();

		static void OnClose();

		// Change the shader data if needed
		static void Update();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStarted().Register<&PBR::Init>();
			EngineEvents::OnEngineStop().Register<&PBR::OnClose>();
			EngineEvents::OnPreUpdate().Register<&PBR::Update>();
		});
	};
}