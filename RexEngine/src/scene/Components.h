#pragma once

#include <memory>

#include "../rendering/Shader.h"
#include "../rendering/Mesh.h"
#include "../math/Vectors.h"
#include "../math/Quaternion.h"
#include "../math/Matrix.h"

#include "Entity.h"

namespace RexEngine
{
	struct MeshRendererComponent
	{
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Mesh> mesh;
		unsigned char priority = 0;
	};

	struct TransformComponent
	{
		Vector3 position;
		Vector3 scale;
		Quaternion rotation;

		Entity parent;

		Matrix4 GetTransform() const
		{
			return Matrix4::MakeTransform(position, rotation, scale);
		}

		Matrix4 GetGlobalTransform() const
		{
			Matrix4 parentMatrix = Matrix4::Identity;
			if (parent && parent.HasComponent<TransformComponent>())
				parentMatrix = parent.GetComponent<TransformComponent>().GetGlobalTransform();

			return parentMatrix * GetTransform();
		}
	};

	struct CameraComponent
	{
		float fov = 70.0f;
		float zNear = 0.001f;
		float zFar = 100.0f;
	};
}