#pragma once

// For external use only, should not be included in RexEngine

// Core
#include "src/core/Log.h"
#include "src/core/Assert.h"
#include "src/core/Guid.h"
#include "src/core/Time.h"
#include "src/core/FileStructure.h"
#include "src/core/Serialization.h"

// Math
#include "src/math/Scalar.h"
#include "src/math/Vectors.h"
#include "src/math/Quaternion.h"
#include "src/math/Matrix.h"

// Scene
#include "src/scene/Scene.h"
#include "src/scene/SceneManager.h"
#include "src/scene/Entity.h"
#include "src/scene/Components.h"

// Rendering
#include "src/rendering/Shader.h"
#include "src/rendering/Mesh.h"
#include "src/rendering/RenderQueue.h"
#include "src/rendering/ForwardRenderer.h"
#include "src/rendering/Shapes.h"
#include "src/rendering/PBR.h"

// Window
#include "src/window/Window.h"

// Inputs
#include "src/inputs/Inputs.h"
#include "src/inputs/Keyboard.h"
#include "src/inputs/Mouse.h"
#include "src/inputs/Input.h"

// Scripting
#include "src/scripting/ScriptEngine.h"
#include "src/scripting/ScriptHost.h"

// Events
#include "src/events/Event.h"
#include "src/events/EngineEvents.h"

// Assets
#include "src/assets/AssetManager.h"

// Utils
#include "src/utils/Concepts.h"
#include "src/utils/NoDestroy.h"
#include "src/utils/StaticConstructor.h"
#include "src/utils/StringHelper.h"