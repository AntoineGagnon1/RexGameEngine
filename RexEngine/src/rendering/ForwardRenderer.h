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

		struct PointLight
		{
			Vector3 Pos;
			float padding; // Empty padding to match opengl
			Vector3 Color;

			PointLight(Vector3 pos, Vector3 color)
				: Pos(pos), padding(0.0f), Color(color)
			{ }
		};

	public:

		// Render a scene using a camera
		static void RenderScene(Asset<Scene> scene, const CameraComponent& camera);

		static RenderApi::BufferID GetSceneDataUniforms();
		static RenderApi::BufferID GetPointLightsBuffer();

	private:

		// Size of the current PointLights buffer, this will change if more are needed
		inline static uint32_t PointLightsMax = 0;

		RE_STATIC_CONSTRUCTOR({
			Shader::RegisterParserUsing("SceneData", "layout (std140, binding = 1) uniform SceneData{ mat4 worldToView; mat4 viewToScreen; vec3 cameraPos; }; ");
			Shader::RegisterParserUsing("Lighting", R"(
struct PointLight
{
	vec3 Pos;
	vec3 Color;
};
layout (std140, binding = 3) uniform PointLightsData
{
    uint PointLightCount;
	PointLight PointLights[];
};
)");
		});
	};
}