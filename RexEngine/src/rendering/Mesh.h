#pragma once

#include "RenderApi.h"
#include "../math/Vectors.h"

namespace RexEngine
{

	class Mesh
	{
	public:
		// Type of vertex attributes : 
		// Position(Vertex) and Normal : Vector3
		// TexCoords : Vector4, if less fill as : (0,0,0,1)

		Mesh(std::span<Vector3> vertices, std::span<unsigned int> indices, std::span<Vector3> normals = {});
		
		// TODO : Load mesh from file

		void Bind();

		auto GetID() { return m_vertexAttributes; }
		size_t GetIndexCount() { return m_indices.size(); }


	private:

		std::vector<uint8_t> m_vertexData;
		std::vector<unsigned int> m_indices; // TODO : make int and short mode ?
		bool m_hasNormals;

		// TODO : make mesh.static option
		RenderApi::BufferID m_vertexBuffer = 0;
		RenderApi::BufferID m_indexBuffer = 0;
		RenderApi::VertexAttribID m_vertexAttributes = 0;
	};

}