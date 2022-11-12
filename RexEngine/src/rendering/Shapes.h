#pragma once

#include <vector>
#include <cmath>
#include <memory>

#include "Mesh.h"
#include "../math/Vectors.h"
#include "../utils/NoDestroy.h"

namespace RexEngine::Shapes
{
	// Cube
	constexpr Vector3 CubeVertices[] = {
		{-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, { 1 ,1,-1}, { 1,-1,-1}, {-1,-1,-1},
		{ 1,-1,-1}, { 1, 1,-1}, { 1, 1, 1}, { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1},
		{-1,-1, 1}, {-1, 1, 1}, {-1, 1,-1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1},
		{ 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1},
		{-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1, 1}, { 1, 1,-1}, {-1, 1,-1},
		{-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1}
	};

	constexpr unsigned int CubeIndices[] = {
		0,1,2, 3,4,5, 6,7,8, 9,10,11, 12,13,14, 15,16,17, 18,19,20, 21,22,23, 24,25,26, 27,28,29, 30,31,32, 33,34,35
	};

	inline std::shared_ptr<Mesh> GetCubeMesh()
	{
		static const NoDestroy<std::shared_ptr<Mesh>> mesh(std::make_shared<Mesh>(CubeVertices, CubeIndices));
		return *mesh;
	}

	namespace Internal{
		inline void MakeSphereMesh(std::vector<Vector3>& vertices, std::vector<Vector3>& normals, std::vector<unsigned int>& indices)
		{ // From : http://www.songho.ca/opengl/gl_sphere.html
			const float PI = 3.1415926f;
			const int stackCount = 64;
			const int sectorCount = 64;
			const float radius = 1.0f;

			float x, y, z, xy;                              // vertex position
			float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
			float s, t;                                     // vertex texCoord

			float sectorStep = 2 * PI / sectorCount;
			float stackStep = PI / stackCount;
			float sectorAngle, stackAngle;

			for (int i = 0; i <= stackCount; ++i)
			{
				stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
				xy = radius * cosf(stackAngle);             // r * cos(u)
				z = radius * sinf(stackAngle);              // r * sin(u)

				// add (sectorCount+1) vertices per stack
				// the first and last vertices have same position and normal, but different tex coords
				for (int j = 0; j <= sectorCount; ++j)
				{
					sectorAngle = j * sectorStep;           // starting from 0 to 2pi

					// vertex position (x, y, z)
					x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
					y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
					vertices.push_back(Vector3(x, y, z));

					// normalized vertex normal (nx, ny, nz)
					nx = x * lengthInv;
					ny = y * lengthInv;
					nz = z * lengthInv;
					normals.push_back(Vector3(nx, ny, nz));

					// vertex tex coord (s, t) range between [0, 1]
					s = (float)j / sectorCount;
					t = (float)i / stackCount;
					//texCoords.push_back(Vector2(s,t));
				}
			}

			int k1, k2;
			for (int i = 0; i < stackCount; ++i)
			{
				k1 = i * (sectorCount + 1);     // beginning of current stack
				k2 = k1 + sectorCount + 1;      // beginning of next stack

				for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
				{
					// 2 triangles per sector excluding first and last stacks
					// k1 => k2 => k1+1
					if (i != 0)
					{
						indices.push_back(k1);
						indices.push_back(k2);
						indices.push_back(k1 + 1);
					}

					// k1+1 => k2 => k2+1
					if (i != (stackCount - 1))
					{
						indices.push_back(k1 + 1);
						indices.push_back(k2);
						indices.push_back(k2 + 1);
					}
				}
			}
		}
	}

	inline std::shared_ptr<Mesh> GetSphereMesh()
	{
		static std::vector<Vector3> vertices, normals;
		static std::vector<unsigned int> indices;

		// Run this once
		static const auto _ = [] { // Save to static var to only run the lamda once
 			Internal::MakeSphereMesh(vertices, normals, indices);
			return 1;
		}();

		static const NoDestroy<std::shared_ptr<Mesh>> mesh(std::make_shared<Mesh>(vertices, indices, normals));
		return *mesh;
	}
}