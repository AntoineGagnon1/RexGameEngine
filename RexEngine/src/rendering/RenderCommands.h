#pragma once

#include "../math/Matrix.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "UniformBlock.h"

// Some default rendercommands

namespace RexEngine
{
	struct ModelUniforms
	{
		Matrix4 modelToWorld;

	private:
		RE_STATIC_CONSTRUCTOR({
			const int location = UniformBlocks::ReserveBlock<ModelUniforms>("ModelData");
			Shader::RegisterParserUsing("ModelData", std::format("layout (std140, binding = {}) uniform ModelData{{ mat4 modelToWorld; }}; ", location));
		});
	};

	// Sorted by Material and mesh
	struct OpaqueRenderCommand
	{
		Matrix4 modelMatrix;

		std::shared_ptr<Material> material;
		std::shared_ptr<Mesh> mesh;


		OpaqueRenderCommand(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh, Matrix4 modelMatrix)
			: material(material), mesh(mesh), modelMatrix(modelMatrix)
		{ }

		OpaqueRenderCommand() : OpaqueRenderCommand(nullptr, nullptr, Matrix4::Identity)
		{}

		void Render(const OpaqueRenderCommand& last) const;

		friend bool operator<(const OpaqueRenderCommand& left, const OpaqueRenderCommand& right);
	};

	// Sorted by distance to the camera
	struct TransparentRenderCommand
	{
		Matrix4 modelMatrix;

		std::shared_ptr<Material> material;
		std::shared_ptr<Mesh> mesh;
		float distanceToCamera;


		TransparentRenderCommand(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh, Matrix4 modelMatrix, Vector3 cameraPos)
			: material(material), mesh(mesh), modelMatrix(modelMatrix), distanceToCamera(Vector3(cameraPos - modelMatrix.Position()).Magnitude())
		{ }

		TransparentRenderCommand() : TransparentRenderCommand(nullptr, nullptr, Matrix4::Identity, {0,0,0})
		{}

		void Render(const TransparentRenderCommand& last) const;

		friend bool operator<(const TransparentRenderCommand& left, const TransparentRenderCommand& right);
	};
}