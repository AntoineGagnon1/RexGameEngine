#pragma once

#include <RexEngine.h>
#include "../project/Project.h"

namespace RexEditor
{
	class EditorEvents
	{
	public:

			RE_DECL_EVENT(OnEditorStart) // Should only be called once, when the editor is started
			RE_DECL_EVENT(OnEditorStarted) // Called after OnEditorStart and OnEngineStart
			RE_DECL_EVENT(OnEditorStop) // Should only be called once, before the app closes

			RE_DECL_EVENT(OnLoadProject, Project) // Called after a project is loaded, argument is the project that was loaded

			RE_DECL_EVENT(OnOpenScene, Asset<Scene>) // Called after a scene is opened, argument is the scene that was opened

			RE_DECL_EVENT(OnUI, float) // Called when the UI should be rendered, argument is the deltaTime to use
			RE_DECL_EVENT(OnGizmos)
	};
}