#pragma once

#include <RexEngine.h>
#include "../core/EditorEvents.h"

namespace RexEditor 
{
	// Gizmos are editor only graphics elements, like Billboards for the camera and light icons
	class Gizmos
	{
	public:

		// Should only be called in the OnGizmos event
		static void Billboard(const RexEngine::Texture& texture, const RexEngine::Vector3& position, const RexEngine::Vector3& size = {1,1,1});

		// This will call the OnGizmos event, and empty the render queue
		static void DrawGizmos(const TransformComponent& camera);

	private:
		inline static std::shared_ptr<Material> s_billboardMaterial;

		static void Init()
		{
			s_billboardMaterial = std::make_shared<Material>(Asset<Shader>(Guid::Generate(), Shader::FromFile("assets/shaders/Billboard.shader")));
		}

		static void Close()
		{
			s_billboardMaterial = nullptr;
		}

		static void DefaultGizmos();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStarted().Register<&Gizmos::Init>();
			EngineEvents::OnEngineStop().Register<&Gizmos::Close>();

			EditorEvents::OnGizmos().Register<&Gizmos::DefaultGizmos>();
		})
	};
}