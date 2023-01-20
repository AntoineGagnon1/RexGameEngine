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

		// Make a mesh from some obj data
		// Supported face types : Triangles
		// Supported data : Vertex position, normals and indices
		static std::shared_ptr<Mesh> FromObj(std::istream& data);

		void Bind() const;
		inline static void UnBind() { RenderApi::BindVertexAttributes(0); }

		auto GetID() const { return m_vertexAttributes; }
		size_t GetIndexCount() const { return m_indices.size(); }
		size_t GetVertexCount() const 
		{ 
			return m_vertexData.size() / sizeof(Vector3) 
				+ (m_hasNormals ? sizeof(Vector3) : 0
				+ (m_hasUVs ? sizeof(Vector2) : 0));
		}

		bool HasNormals() const { return m_hasNormals; }
		bool HasUVs() const { return m_hasUVs; }

		template<typename Archive>
		inline static std::shared_ptr<Mesh> LoadFromAssetFile([[maybe_unused]]Guid _, [[maybe_unused]]const Archive& metaDataArchive, std::istream& assetFile)
		{
			return FromObj(assetFile);
		}

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