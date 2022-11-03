#pragma once

#include "../scene/Scene.h"
#include "../scene/Components.h"

namespace RexEngine
{
	class ForwardRenderer
	{
	public:

		// Uniform block sent to the shaders
		struct SceneDataUniforms
		{
			Matrix4 viewMatrix;
			Matrix4 projectionMatrix;

		private:
			// Only used to register to the shader parser, see definition at the bottom of the file
			static int RegisterParser;
		};

	public:

		// Render a scene using a camera
		static void RenderScene(Scene& scene, const CameraComponent& camera);

	private:

		static RenderApi::BufferID GetSceneDataUniforms();
	};


	inline int ForwardRenderer::SceneDataUniforms::RegisterParser = [] 
	{
		Shader::RegisterParserUsing("SceneData", "layout (std140, binding = 1) uniform SceneData{ mat4 viewMatrix; mat4 projectionMatrix; }; ");
		return 0;
	}();
}