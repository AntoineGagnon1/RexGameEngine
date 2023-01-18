#include <REPch.h>
#include "Mesh.h"

#include "Shader.h"
#include "utils/TupleHash.h"

namespace RexEngine
{

	Mesh::Mesh(std::span<const Vector3> vertices, std::span<const unsigned int> indices, std::span<const Vector3> normals, std::span<const Vector2> uvs)
	{
		m_hasNormals = normals.size() > 0;
		m_hasUVs = uvs.size() > 0;

		RE_ASSERT(vertices.size() == normals.size() || !m_hasNormals, "Mesh normal count was not the same as vertex count !");
		RE_ASSERT(vertices.size() == uvs.size() || !m_hasUVs, "Mesh uvs count was not the same as vertex count !");
		
		m_vertexData.reserve(vertices.size_bytes() + normals.size_bytes() + uvs.size_bytes());

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

			if (m_hasUVs)
			{
				for (int s = 0; s < sizeof(Vector2); s++)
					m_vertexData.push_back(((uint8_t*)&uvs[i])[s]);
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
		if(m_hasUVs)
			attributes.push_back({ RenderApi::VertexAttributeType::Float2 , Shader::UVLocation }); // UV


		m_vertexAttributes = RenderApi::MakeVertexAttributes(std::span(attributes), m_vertexBuffer, m_indexBuffer);
	}

	Mesh::~Mesh()
	{
		RenderApi::DeleteVertexAttributes(m_vertexAttributes);
		RenderApi::DeleteBuffer(m_vertexBuffer);
		RenderApi::DeleteBuffer(m_indexBuffer);
	}

	std::shared_ptr<Mesh> Mesh::FromObj(std::istream& data)
	{
		std::vector<Vector3> vertices;
		std::vector<Vector3> normals = {Vector3(0,0,0)}; // Add a null normal, if the mesh has none this will prevent an out of bounds access
		std::vector<Vector2> uvs = {Vector2(0,0)}; // Add a null uv, if the mesh has none this will prevent an out of bounds access
		//                    <vertex, normal, uv>
		std::vector<std::tuple<int,int, int>> faces;

		// Go line by line
		std::string line;
		while (std::getline(data, line))
		{
			if (line.empty())
				continue;
			
			auto args = StringHelper::Split(line, ' ');

			if (args[0] == "v")
			{ // vertex
				vertices.push_back(Vector3(std::stof(args[1]), std::stof(args[2]), std::stof(args[3])));
			}
			else if (args[0] == "vn")
			{ // normal
				normals.push_back(Vector3(std::stof(args[1]), std::stof(args[2]), std::stof(args[3])).Normalized());
			}
			else if (args[0] == "vt")
			{
				uvs.push_back(Vector2(std::stof(args[1]), std::stof(args[2])));
			}
			else if (args[0] == "f")
			{ // face
				for (int i = 0; i < 3; i++)
				{ // Get each component : vertexIndex/textureIndex/normalIndex
					auto components = StringHelper::Split(args[i + 1], '/');

					int v = std::stoi(components[0]) - 1;
					int uv = 0;
					int n = 0;

					if (components.size() >= 2 && !components[1].empty())
						uv = std::stoi(components[1]); // not -1 because of the default normal
					if (components.size() >= 3 && !components[2].empty())
						n = std::stoi(components[2]); // not -1 because of the default uv

					faces.push_back({ v, n, uv });
				}
			}
		}

		// Loop all the faces and make the vertex buffer

		// This will store the index of each unique vertex
		//							<<vertex, normal, uv>, meshIndex>
		std::unordered_map<std::tuple<int, int, int>, int> vertexIndex;
		std::vector<unsigned int> meshIndices;
		std::vector<Vector3> meshVertices;
		std::vector<Vector3> meshNormals;
		std::vector<Vector2> meshUVs;
		
		for (int i = 0; i < faces.size(); i++)
		{
			auto&[vIndex, nIndex, uvIndex] = faces[i];
			auto index = vertexIndex.find(faces[i]);
			if (index != vertexIndex.end())
				meshIndices.push_back((unsigned int)index->second);
			else
			{
				meshIndices.push_back((unsigned int)vertexIndex.size());
				vertexIndex.insert({ faces[i], meshVertices.size() });

				meshVertices.push_back(vertices[vIndex]);
				if(normals.size() > 1)
					meshNormals.push_back(normals[nIndex]);
				if (uvs.size() > 1)
					meshUVs.push_back(uvs[uvIndex]);
			}
		}
		
		return std::make_shared<Mesh>(meshVertices, meshIndices, meshNormals, meshUVs);
	}

	void Mesh::Bind() const
	{
		RenderApi::BindVertexAttributes(m_vertexAttributes);
		RenderApi::BindBuffer(m_indexBuffer, RenderApi::BufferType::Indice);
	}

}