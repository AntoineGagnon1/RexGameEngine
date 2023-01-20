#include <REPch.h>
#include "RenderQueue.h"


namespace
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
}


namespace RexEngine
{
	bool operator<(const RenderCommand& left, const RenderCommand& right)
	{
		// Sorting order from :
		// https://computergraphics.stackexchange.com/questions/37/what-is-the-cost-of-changing-state/46#46

		bool returnValue;

		if (CompareSmaller<char>(left.material->GetShader()->Priority(), right.material->GetShader()->Priority(), returnValue))
			return returnValue;

		if (CompareSmaller<std::shared_ptr<Material>>(left.material, right.material, returnValue))
			return returnValue;

		if (CompareSmaller<std::shared_ptr<Mesh>>(left.mesh, right.mesh, returnValue))
			return returnValue;

		return false; // Equal
	}



	void RenderQueue::AddCommand(RenderCommand command)
	{
		m_renderCommands.push_back(command);
	}

	void RenderQueue::ExecuteCommands()
	{
		// Sort RenderCommands to minimize the amount of state changes
		std::sort(m_renderCommands.begin(), m_renderCommands.end());

		// Used to detect state changes
		RenderCommand tempCommand{std::shared_ptr<Material>(), std::shared_ptr<Mesh>(), Matrix4::Identity};
		RenderApi::SetCullingMode(RenderApi::CullingMode::Front);
		
		// Execute the commands
		for (auto&& command : m_renderCommands)
		{
			if (command.material != tempCommand.material)
			{ // The shader changed
				tempCommand.material = command.material;
				command.material->Bind();
			}

			if (command.mesh != tempCommand.mesh)
			{ // The vertex data changed
				tempCommand.mesh = command.mesh;
				command.mesh->Bind();
			}

			// Update the model data
			//Matrix3 normalMatrix = Matrix3(command.modelMatrix.Inversed().Transposed()); // Model to world Normals matrix
			ModelUniforms modelData {command.modelMatrix };
			RenderApi::SubBufferData(GetModelUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(ModelUniforms), &modelData);

			RenderApi::DrawElements(tempCommand.mesh->GetIndexCount());
		}

		m_renderCommands.clear();
	}

	RenderApi::BufferID RenderQueue::GetModelUniforms()
	{
		static RenderApi::BufferID uniforms = []() {
			auto buf = RenderApi::MakeBuffer();
			auto data = ModelUniforms();
			RenderApi::SetBufferData(buf, RenderApi::BufferType::Uniforms, RenderApi::BufferMode::Dynamic, (uint8_t*)&data, sizeof(ModelUniforms));
			RenderApi::BindBufferBase(buf, 2);

			return buf;
		}();

		return uniforms;
	}
}