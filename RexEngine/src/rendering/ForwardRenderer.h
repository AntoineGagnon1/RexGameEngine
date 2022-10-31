#pragma once

#include "../scene/Scene.h"
#include "../scene/Components.h"

namespace RexEngine
{
	class ForwardRenderer
	{
	public:

		// Render a scene using a camera
		static void RenderScene(Scene& scene, const CameraComponent& camera);

	private:

	};
}