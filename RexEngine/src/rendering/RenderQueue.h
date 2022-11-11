#pragma once

#include <vector>

#include "RenderApi.h"
#include "Shader.h"
#include "Mesh.h"

namespace RexEngine
{
	struct RenderCommand
	{
		Matrix4 modelMatrix;

		std::shared_ptr<Shader> shader; // TODO : use smaller types ?
		std::shared_ptr<Mesh> mesh;
		unsigned char priority;
		RenderApi::CullingMode cullingMode; // Not sorted
		// TODO :
		// Output buffer id
		// Textures


		RenderCommand(std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh, Matrix4 modelMatrix, RenderApi::CullingMode cullingMode, unsigned char priority = 0)
			: shader(shader), mesh(mesh), priority(priority), modelMatrix(modelMatrix), cullingMode(cullingMode)
		{ }


		friend bool operator<(const RenderCommand& left, const RenderCommand& right);
	};

	class RenderQueue
	{
	public:
		struct ModelUniforms
		{
			Matrix4 modelToWorld;

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
		Shader::RegisterParserUsing("ModelData", "layout (std140, binding = 2) uniform ModelData{ mat4 modelToWorld; }; ");
		return 0;
	}();
}