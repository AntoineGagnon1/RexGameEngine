#pragma once

#include <vector>

#include "RenderApi.h"
#include "Mesh.h"
#include "Material.h"

namespace RexEngine
{
	struct RenderCommand
	{
		Matrix4 modelMatrix;

		std::shared_ptr<Material> material;
		std::shared_ptr<Mesh> mesh;

		RenderCommand(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh, Matrix4 modelMatrix)
			: material(material), mesh(mesh), modelMatrix(modelMatrix)
		{ }

		friend bool operator<(const RenderCommand& left, const RenderCommand& right);
	};

	class RenderQueue
	{
	public:
		struct ModelUniforms
		{
			Matrix4 modelToWorld;
		};

	public:

		static void AddCommand(RenderCommand command);

		static void ExecuteCommands();

		static RenderApi::BufferID GetModelUniforms();
	private:
		inline static std::vector<RenderCommand> m_renderCommands;

		RE_STATIC_CONSTRUCTOR({
			Shader::RegisterParserUsing("ModelData", "layout (std140, binding = 2) uniform ModelData{ mat4 modelToWorld; }; ");
		});
	};
}