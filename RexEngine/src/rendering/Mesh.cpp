#include <REPch.h>
#include "Mesh.h"

#include "Shader.h"

namespace RexEngine
{

	Mesh::Mesh(std::span<const Vector3> vertices, std::span<const unsigned int> indices, std::span<const Vector3> normals)
	{
		m_hasNormals = normals.size() > 0;
		RE_ASSERT(vertices.size() == normals.size() || !m_hasNormals, "Mesh normal count was not the same as vertex count !");
		
		m_vertexData.reserve(vertices.size_bytes() + normals.size_bytes());

		// Make the vertex data buffer
		for (int i = 0; i < vertices.size(); i++)
		{
			for (int s = 0; s < sizeof(Vector3); s++)
				m_vertexData.push_back(((uint8_t*)&vertices[i])[s]);

			if (m_hasNormals)
			{
				for (int s = 0; s < sizeof(Vector3); s++)
					m_vertexData.push_back(((uint8_t*)&normals[i])[s]);
			}
		}

		// Indices
		m_indices.assign(indices.begin(), indices.end());

		// Tell the RenderApi
		m_vertexBuffer = RenderApi::MakeBuffer();
		RenderApi::SetBufferData(m_vertexBuffer, RenderApi::BufferType::Vertex, RenderApi::BufferMode::Dynamic, std::span(m_vertexData));

		m_indexBuffer = RenderApi::MakeBuffer();
		RenderApi::SetBufferData(m_indexBuffer, RenderApi::BufferType::Indice, RenderApi::BufferMode::Dynamic, std::span(m_indices));
		
		// Make the Vertex Attributes
		std::vector<std::tuple<RenderApi::VertexAttributeType, int>> attributes;
		attributes.push_back({ RenderApi::VertexAttributeType::Float3 , Shader::PositionLocation}); // Position
		if(m_hasNormals)
			attributes.push_back({ RenderApi::VertexAttributeType::Float3 , Shader::NormalLocation }); // Normal

		m_vertexAttributes = RenderApi::MakeVertexAttributes(std::span(attributes), m_vertexBuffer, m_indexBuffer);
	}

	Mesh::~Mesh()
	{
		RenderApi::DeleteVertexAttributes(m_vertexAttributes);
		RenderApi::DeleteBuffer(m_vertexBuffer);
		RenderApi::DeleteBuffer(m_indexBuffer);
	}

	void Mesh::Bind() const
	{
		RenderApi::BindVertexAttributes(m_vertexAttributes);
		RenderApi::BindBuffer(m_indexBuffer, RenderApi::BufferType::Indice);
	}

}