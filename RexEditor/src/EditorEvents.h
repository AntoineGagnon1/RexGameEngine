#pragma once

#include <RexEngine.h>

namespace RexEditor
{
	class EditorEvents
	{
	public:

			RE_DECL_EVENT(OnEditorStart) // Should only be called once, when the editor is started

			RE_DECL_EVENT(OnEditorStop) // Should only be called once, before the app closes
	};
}