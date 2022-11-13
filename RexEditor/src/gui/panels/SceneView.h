#pragma once

#include <RexEngine.h>
#include <map>
#include <memory>
#include <string>

#include "../Panel.h"
#include "../Gui.h"

#include "../../project/Project.h"

namespace RexEditor
{
	class SceneView : public Panel
	{
	public:
		SceneView() : Panel("Scene View"),
			m_viewTexture(RexEngine::RenderApi::TextureTarget::Texture2D, RexEngine::RenderApi::PixelFormat::RGB, { 0,0 }, NULL, RexEngine::RenderApi::PixelFormat::RGB, RexEngine::RenderApi::PixelType::UByte),
			m_viewDepth(RexEngine::RenderApi::PixelType::Depth, { 0,0 }),
			m_roll(0.0f), m_pitch(0.0f)
		{
			m_viewBuffer.BindTexture(m_viewTexture, RexEngine::RenderApi::FrameBufferTextureType::Color);
			m_viewBuffer.BindRenderBuffer(m_viewDepth, RexEngine::RenderApi::FrameBufferTextureType::Depth);

			m_inputs.insert({"Capture", std::move(std::make_unique<MouseButtonInput>(MouseButton::Right)) });
			m_inputs.insert({"MoveForward", std::move(std::make_unique<KeyboardInput>(KeyCode::W, KeyCode::S)) });
			m_inputs.insert({"MoveRight", std::move(std::make_unique<KeyboardInput>(KeyCode::D, KeyCode::A)) });
			m_inputs.insert({"MoveUp", std::move(std::make_unique<KeyboardInput>(KeyCode::Space, KeyCode::LeftShift)) });
			m_inputs.insert({"LookRight", std::move(std::make_unique<MouseInput>(MouseInputType::DeltaX)) });
			m_inputs.insert({"LookUp", std::move(std::make_unique<MouseInput>(MouseInputType::DeltaY)) });
		}

	protected:
		virtual void OnResize(RexEngine::Vector2Int oldSize, RexEngine::Vector2Int newSize) override
		{
			m_viewTexture.SetData(newSize, NULL, RexEngine::RenderApi::PixelFormat::RGB, RexEngine::RenderApi::PixelType::UByte);
			m_viewDepth.SetSize(newSize);
		}

		virtual void OnGui(float deltaTime) override
		{
			// Only move if focused
			if (IsFocused())
			{
				// Update the inputs
				for (auto&& pair : m_inputs)
					pair.second->PollInputs();

				// Free/capture the mouse if needed
				if(m_inputs["Capture"]->IsJustDown() && IsFocused())
					RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Locked);
				else if (m_inputs["Capture"]->IsJustUp() && IsFocused())
					RexEngine::Cursor::SetCursorMode(RexEngine::CursorMode::Free);

				// FPS Controls
				if (m_inputs["Capture"]->IsDown())
				{
					m_cameraTransform.position += m_cameraTransform.Forward() * (m_inputs["MoveForward"]->GetValue() * m_moveSpeed * Time::DeltaTime());
					m_cameraTransform.position += m_cameraTransform.Right() * (m_inputs["MoveRight"]->GetValue() * m_moveSpeed * Time::DeltaTime());
					m_cameraTransform.position += m_cameraTransform.Up() * (m_inputs["MoveUp"]->GetValue() * m_moveSpeed * Time::DeltaTime());

					m_roll += m_rotationSpeed * -m_inputs["LookRight"]->GetValue() * Time::DeltaTime();
					m_pitch += m_rotationSpeed * -m_inputs["LookUp"]->GetValue() * Time::DeltaTime();

					m_pitch = Scalar::Clamp(m_pitch, -89.999f, 89.999f);

					m_cameraTransform.rotation = Quaternion::AngleAxis(m_roll, Directions::Up);
					m_cameraTransform.rotation.Rotate(m_pitch, m_cameraTransform.Right());
				}
			}

			// Tell the engine timer that a new frame has begun
			RexEngine::Time::StartNewFrame();

			// Create the camera object using the cached components, TODO : do this on scene changed
			auto camera = Project::CurrentScene.CreateEntity();
			auto& cameraComponent = camera.AddComponent<RexEngine::CameraComponent>(m_editorCamera);
			camera.AddComponent<RexEngine::TransformComponent>(m_cameraTransform);

			m_viewBuffer.Bind();

			auto oldViewportSize = RexEngine::RenderApi::GetViewportSize(); // Cache the size to revert at the end
			RexEngine::RenderApi::SetViewportSize(PanelSize());

			RexEngine::RenderApi::ClearColorBit();
			RexEngine::RenderApi::ClearDepthBit();

			// Render the scene from the pov of the editor camera
			RexEngine::ForwardRenderer::RenderScene(Project::CurrentScene, cameraComponent);
			Gui::DrawFullWindowTexture(m_viewTexture);

			// Revert back to the cached states
			m_viewBuffer.UnBind();
			RexEngine::RenderApi::SetViewportSize(oldViewportSize);
			Project::CurrentScene.DestroyEntity(camera);
		}

	private:
		RexEngine::CameraComponent m_editorCamera;
		RexEngine::TransformComponent m_cameraTransform;
		float m_roll, m_pitch;
		const float m_moveSpeed = 1.0f;
		const float m_rotationSpeed = 40000.0f;

		RexEngine::FrameBuffer m_viewBuffer;
		RexEngine::Texture m_viewTexture;
		RexEngine::RenderBuffer m_viewDepth;

		std::map<std::string, std::unique_ptr<RexEngine::Input>> m_inputs;
	};
}
