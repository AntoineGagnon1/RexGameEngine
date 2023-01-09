#pragma once

#include "../scene/Scene.h"
#include "../scene/Components.h"

#include "FrameBuffer.h"

namespace RexEngine
{
	class ForwardRenderer
	{
	public:

		static constexpr int MaxLights = 32; // Max lights for each type

		// Uniform block sent to the shaders for the scene data
		struct SceneDataUniforms
		{
			Matrix4 worldToView;
			Matrix4 viewToScreen;
			Vector3 cameraPos;
		};

		struct LightData
		{
			Vector4 Pos; // If the w component == 0, the light is directional and if == 1 it is a point light
			Vector3 Color;
			float padding; // To match opengl

			LightData(Vector3 pos, Vector3 color, bool directional)
				: Pos(pos, directional ? 0.0f : 1.0f), Color(color), padding(0.0f)
			{ }
		};

		struct SpotLightData
		{
			Vector3 Dir;
			float padding1; // to match opengl
			Vector3 Pos;
			float padding2; // to match opengl
			Vector3 Color;
			float padding3; // to match opengl
			float CutOff;
			float OuterCutOff;
			float padding4; // to match opengl
			float padding5; // to match opengl

			SpotLightData(Vector3 pos, Vector3 dir, Vector3 color, float cutOff, float outerCutOff)
				: Dir(dir), padding1(0.0f), Pos(pos), padding2(0.0f), Color(color), padding3(0.0f), CutOff(cutOff), OuterCutOff(outerCutOff), padding4(0.0f), padding5(0.0f)
			{ }
		};

	public:

		// Render a scene using a camera
		static void RenderScene(Asset<Scene> scene, const CameraComponent& camera);

		static RenderApi::BufferID GetSceneDataUniforms();
		static RenderApi::BufferID GetLightsBuffer();
		static RenderApi::BufferID GetSpotLightsBuffer();

	private:

		// Size of the current Lights buffer, this will change if more are needed
		inline static uint32_t LightsMax = 0;
		inline static uint32_t SpotLightsMax = 0;

		RE_STATIC_CONSTRUCTOR({
			Shader::RegisterParserUsing("SceneData", "layout (std140, binding = 1) uniform SceneData{ mat4 worldToView; mat4 viewToScreen; vec3 cameraPos; }; ");
			
			Shader::RegisterParserUsing("Lighting", R"(
struct LightData { vec4 Pos; vec3 Color; };
layout (std140, binding = 3) uniform LightsData { uint LightCount; LightData Lights[]; };
struct SpotLightData { vec4 Dir; vec4 Pos; vec4 Color; float CutOff; float OuterCutOff; };
layout (std140, binding = 4) uniform SpotLightsData { uint SpotLightCount; SpotLightData SpotLights[]; };
)");
		});
	};
}