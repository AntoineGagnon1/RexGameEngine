#pragma once

#include <vector>

#include "RenderApi.h"

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

		RenderCommand(RenderApi::ShaderID shader, RenderApi::VertexAttribID vertexData, size_t indiceCount, unsigned char priority = 0)
			: shader(shader), vertexData(vertexData), priority(priority), indiceCount(indiceCount)
		{ }


		friend bool operator<(const RenderCommand& left, const RenderCommand& right);
	};

	class RenderQueue
	{
	public:

		static void AddCommand(RenderCommand command);

		static void ExecuteCommands();

	private:
		inline static std::vector<RenderCommand> m_renderCommands;
	};
}