#pragma once

#include "../scene/Scene.h"
#include "../scene/Components.h"

#include "FrameBuffer.h"

namespace RexEngine
{
	class ForwardRenderer
	{
	public:

		// Uniform block sent to the shaders for the scene data
		struct SceneDataUniforms
		{
			Matrix4 worldToView;
			Matrix4 viewToScreen;
			Vector3 cameraPos;

		private:
			// Only used to register to the shader parser, see definition at the bottom of the file
			static int RegisterParser;
		};


		// The lighting related uniform block
		struct LightingUniforms
		{
			Vector3 lightPos; // TODO : multiple lights
			float padding; // Padding to match opengl
			Vector3 lightColor;

		private:
			// Only used to register to the shader parser, see definition at the bottom of the file
			static int RegisterParser;
		};

	public:

		// Render a scene using a camera
		static void RenderScene(Scene scene, const CameraComponent& camera);

	private:

		static RenderApi::BufferID GetSceneDataUniforms();
		static RenderApi::BufferID GetLightingUniforms();
	};

	inline int ForwardRenderer::SceneDataUniforms::RegisterParser = [] 
	{
		Shader::RegisterParserUsing("SceneData", "layout (std140, binding = 1) uniform SceneData{ mat4 worldToView; mat4 viewToScreen; vec3 cameraPos; }; ");
		return 0;
	}();

	inline int ForwardRenderer::LightingUniforms::RegisterParser = []
	{
		Shader::RegisterParserUsing("Lighting", "layout (std140, binding = 3) uniform Lighting{ vec3 lightPos; vec3 lightColor; }; ");
		return 0;
	}();
}