#include <REPch.h>
#include "RenderCommands.h"
#include "RenderQueue.h"

namespace RexEngine::Internal
{
	// Returns true if the values are not equal
	template<typename T>
	inline static bool CompareSmaller(T left, T right, bool& smaller)
	{
		if (left < right)
		{
			smaller = true;
			return true;
		}
		else if (left > right)
		{
			smaller = false;
			return true;
		}

		return false; // The values are equal
	}

	class RenderCommandsInit
	{
		RE_STATIC_CONSTRUCTOR({
			RexEngine::RenderQueues::AddQueue<OpaqueRenderCommand>("Opaque", 0);
			RexEngine::RenderQueues::AddQueue<TransparentRenderCommand>("Transparent", 1000);
		})
	};
}

namespace RexEngine
{
	bool operator<(const OpaqueRenderCommand& left, const OpaqueRenderCommand& right)
	{
		// Sorting order from :
		// https://computergraphics.stackexchange.com/questions/37/what-is-the-cost-of-changing-state/46#46

		bool returnValue;

		if (Internal::CompareSmaller<char>(left.material->GetShader()->Priority(), right.material->GetShader()->Priority(), returnValue))
			return returnValue;

		if (Internal::CompareSmaller<std::shared_ptr<Material>>(left.material, right.material, returnValue))
			return returnValue;

		if (Internal::CompareSmaller<std::shared_ptr<Mesh>>(left.mesh, right.mesh, returnValue))
			return returnValue;

		return false; // Equal
	}

	void OpaqueRenderCommand::Render(const OpaqueRenderCommand& last) const
	{
		if (material != last.material) // The material changed
			material->Bind();

		if (mesh != last.mesh) // The vertex data changed
			mesh->Bind();

		// Update the model data
		ModelUniforms modelData{ modelMatrix };
		UniformBlocks::GetBlock<ModelUniforms>("ModelData").SetData(modelData);

		RenderApi::DrawElements(mesh->GetIndexCount());
	}

	bool operator<(const TransparentRenderCommand& left, const TransparentRenderCommand& right)
	{
		// Sort based on distance from the camera (further away first)
		return left.distanceToCamera > right.distanceToCamera;
	}

	void TransparentRenderCommand::Render(const TransparentRenderCommand& last) const
	{
		if (material != last.material) // The material changed
			material->Bind();

		if (mesh != last.mesh) // The vertex data changed
			mesh->Bind();

		// Update the model data
		ModelUniforms modelData{ modelMatrix };
		UniformBlocks::GetBlock<ModelUniforms>("ModelData").SetData(modelData);

		RenderApi::DrawElements(mesh->GetIndexCount());
	}
}