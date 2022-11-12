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
		// TexCoords : Vector4, if less fill as : (0,0,0,1) <-- currently vec2

		Mesh(std::span<const Vector3> vertices, std::span<const unsigned int> indices, std::span<const Vector3> normals = {}, std::span<const Vector2> uvs = {});
		~Mesh();
		
		Mesh(const Mesh&) = delete;

		void Bind() const;
		inline static void UnBind() { RenderApi::BindVertexAttributes(0); }

		auto GetID() const { return m_vertexAttributes; }
		size_t GetIndexCount() const { return m_indices.size(); }

	private:

		std::vector<uint8_t> m_vertexData;
		std::vector<unsigned int> m_indices;
		bool m_hasNormals;
		bool m_hasUVs;

		RenderApi::BufferID m_vertexBuffer = 0;
		RenderApi::BufferID m_indexBuffer = 0;
		RenderApi::VertexAttribID m_vertexAttributes = 0;
	};

}