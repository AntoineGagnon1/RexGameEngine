#pragma once

#include <memory>

#include "../rendering/Shader.h"
#include "../rendering/Mesh.h"
#include "../rendering/Cubemap.h"
#include "../math/Vectors.h"
#include "../math/Quaternion.h"
#include "../math/Matrix.h"
#include "../core/Color.h"
#include "../core/Serialization.h"
#include "../assets/AssetManager.h"

#include "Entity.h"

namespace RexEngine
{
	struct MeshRendererComponent
	{
		Asset<Shader> shader;
		Asset<Mesh> mesh;
		RenderApi::CullingMode cullingMode = RenderApi::CullingMode::Front; // When false both sides are rendered
		unsigned char priority = 0;

		template<typename Archive>
		void serialize(Archive& archive) 
		{
			archive(KEEP_NAME(shader), KEEP_NAME(mesh), KEEP_NAME(cullingMode), KEEP_NAME(priority));
		}
	};

	struct TransformComponent
	{
		Vector3 position = Vector3(0,0,0);
		Vector3 scale = Vector3(1,1,1);
		Quaternion rotation = Quaternion(0,0,0,1);

		Entity parent;

		// Local
		Vector3 Forward() const { return rotation.RotateVector(Directions::Forward); }
		// Local
		Vector3 Up() const { return rotation.RotateVector(Directions::Up); }
		// Local
		Vector3 Right() const { return rotation.RotateVector(Directions::Right); }

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

		template<typename Archive>
		void serialize(Archive& archive) 
		{
			archive(KEEP_NAME(position), KEEP_NAME(scale), KEEP_NAME(rotation), KEEP_NAME(parent));
		}
	};

	struct CameraComponent
	{
		float fov = 70.0f;
		float zNear = 0.1f;
		float zFar = 100.0f;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(fov), KEEP_NAME(zNear), KEEP_NAME(zFar));
		}
	};

	struct SkyboxComponent
	{
		Asset<Shader> shader;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(shader));
		}
	};
}