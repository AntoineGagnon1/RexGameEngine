#pragma once

#include <vector>

#include "Mesh.h"
#include "../math/Vectors.h"
#include "../core/NoDestroy.h"

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

	inline const Mesh& GetCubeMesh()
	{
		static const NoDestroy<Mesh> mesh(CubeVertices, CubeIndices);
		return *mesh;
	}

}