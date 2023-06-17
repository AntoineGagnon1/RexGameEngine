#pragma once

// For external use only, should not be included in RexEngine

// Core
#include "src/core/Log.h"
#include "src/core/Guid.h"
#include "src/core/Time.h"
#include "src/core/FileStructure.h"
#include "src/core/Serialization.h"
#include "src/core/Event.h"
#include "src/core/EngineEvents.h"

// Math
#include "src/math/Scalar.h"
#include "src/math/Vectors.h"
#include "src/math/Quaternion.h"
#include "src/math/Matrix.h"

// Scene
#include "src/scene/Scene.h"
#include "src/scene/Entity.h"
#include "src/scene/Components.h"
#include "src/scene/ComponentFactory.h"

// Rendering
#include "src/rendering/Shader.h"
#include "src/rendering/Mesh.h"
#include "src/rendering/RenderQueue.h"
#include "src/rendering/RenderCommands.h"
#include "src/rendering/ForwardRenderer.h"
#include "src/rendering/Shapes.h"
#include "src/rendering/PBR.h"
#include "src/rendering/MSAATexture.h"
#include "src/rendering/Material.h"
#include "src/rendering/shaders/PBRLit.h"
#include "src/rendering/TextureManager.h"

// Window
#include "src/window/Window.h"

// Inputs
#include "src/inputs/Inputs.h"
#include "src/inputs/Keyboard.h"
#include "src/inputs/Mouse.h"
#include "src/inputs/Input.h"

// Scripting
#include "src/scripting/MonoEngine.h"

// Assets
#include "src/assets/AssetManager.h"
#include "src/assets/AssetTypes.h"

// Utils
#include "src/utils/Concepts.h"
#include "src/utils/NoDestroy.h"
#include "src/utils/StaticConstructor.h"
#include "src/utils/StringHelper.h"