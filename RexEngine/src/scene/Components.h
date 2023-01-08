#pragma once

#include <memory>

#include "../rendering/Material.h"
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
	struct TagComponent
	{
		std::string name;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(name));
		}
	};

	struct MeshRendererComponent
	{
		Asset<Material> material;
		Asset<Mesh> mesh;

		template<typename Archive>
		void serialize(Archive& archive) 
		{
			archive(KEEP_NAME(material), KEEP_NAME(mesh));
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

		Vector3 GlobalPosition() const
		{
			auto mat = GetGlobalTransform();
			return mat.Position();
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
		Asset<Material> material;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(material));
		}
	};

	struct PointLightComponent
	{
		Color color;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color));
		}
	};
}