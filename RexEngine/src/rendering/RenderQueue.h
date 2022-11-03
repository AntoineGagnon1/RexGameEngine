#pragma once

#include <vector>

#include "RenderApi.h"
#include "Shader.h"

namespace RexEngine
{
	struct RenderCommand
	{
		// TODO : use smaller types
		RenderApi::ShaderID shader;
		RenderApi::VertexAttribID vertexData;
		unsigned char priority;

		// TODO :
		// Output buffer id
		// Textures

		size_t indiceCount; // How many indices to draw
		Matrix4 modelMatrix;

		RenderCommand(RenderApi::ShaderID shader, RenderApi::VertexAttribID vertexData, size_t indiceCount, Matrix4 modelMatrix, unsigned char priority = 0)
			: shader(shader), vertexData(vertexData), priority(priority), indiceCount(indiceCount), modelMatrix(modelMatrix)
		{ }


		friend bool operator<(const RenderCommand& left, const RenderCommand& right);
	};

	class RenderQueue
	{
	public:
		struct ModelUniforms
		{
			Matrix4 modelMatrix;

		private:
			// Only used to register to the shader parser, see definition at the bottom of the file
			static int RegisterParser;
		};

	public:

		static void AddCommand(RenderCommand command);

		static void ExecuteCommands();

	private:
		static RenderApi::BufferID GetModelUniforms();
	private:
		inline static std::vector<RenderCommand> m_renderCommands;
	};

	inline int RenderQueue::ModelUniforms::RegisterParser = []
	{
		Shader::RegisterParserUsing("ModelData", "layout (std140, binding = 2) uniform ModelData{ mat4 modelMatrix; }; ");
		return 0;
	}();
}