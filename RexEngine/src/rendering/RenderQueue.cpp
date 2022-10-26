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

		if (CompareSmaller<RenderApi::ShaderID>(left.shader, right.shader, returnValue))
			return returnValue;

		if (CompareSmaller<RenderApi::VertexAttribID>(left.vertexData, right.vertexData, returnValue))
			return returnValue;

		if (CompareSmaller<unsigned char>(left.priority, right.priority, returnValue))
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
		RenderCommand tempCommand{RenderApi::InvalidShaderID, RenderApi::InvalidBufferID, 0, 0};
		
		// Execute the commands
		for (auto&& command : m_renderCommands)
		{
			if (command.shader != tempCommand.shader)
			{ // The shader changed
				tempCommand.shader = command.shader;
				RenderApi::BindShader(command.shader);
			}

			if (command.vertexData != tempCommand.vertexData)
			{ // The vertex data changed
				tempCommand.vertexData = command.vertexData;
				RenderApi::BindVertexAttributes(command.vertexData);
			}

			// Already filtered by command.priority
			RenderApi::DrawElements(command.indiceCount);
		}

		m_renderCommands.clear();
	}
}