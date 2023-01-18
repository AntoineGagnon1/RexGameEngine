#include "REDPch.h"

#include "ui/UI.h"
#include "ui/MenuBar.h"
#include "panels/PanelManager.h"
#include "project/ProjectManager.h"
#include "panels/PlayControls.h"

int WinMain()
{
	using namespace RexEngine;
	using namespace RexEditor;

	Window win("RexEditor", 1280, 720, 8);
	win.SetWindowIcon("assets/icons/logo.png");
	win.MakeActive();
	testWin = &win;
	win.SetResizeCallback([](Vector2Int size) {
		RenderApi::SetViewportSize(size);
	});

	EditorEvents::OnEditorStart().Dispatch();

	EngineEvents::OnEngineStart().Dispatch();

	EngineEvents::OnEngineStarted().Dispatch();

	ScriptEngine::LoadAssembly(Dirs::ScriptDir / "Editor" / "RexEditorScript.dll");

	EditorEvents::OnEditorStarted().Dispatch();

	// Temp to save some time while testing
	ProjectManager::Load("../../../RexEditor/Projects/TestProject/TestProject.rexengine");

	Timer editorFrameTime;
	editorFrameTime.Start();
	while (!win.ShouldClose())
	{
		float deltaTime = (float)editorFrameTime.ElapsedSeconds();
		editorFrameTime.Restart();

		EngineEvents::OnPreUpdate().Dispatch();

		if(PanelManager::GetPanel<PlayControlsPanel>()->IsPlaying())
			EngineEvents::OnUpdate().Dispatch(); // Only update if the engine is in play mode
		
		EditorEvents::OnUI().Dispatch(std::forward<float>(deltaTime));

		UI::RenderUI();

		win.SwapBuffers();
	}
	
	EngineEvents::OnEngineStop().Dispatch();
	EditorEvents::OnEditorStop().Dispatch();

	return 0;
}