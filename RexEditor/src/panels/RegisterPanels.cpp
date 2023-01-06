#include "REDPch.h"
#include <RexEngine.h>

#include "PanelManager.h"

#include "NewProject.h"
#include "FileExplorer.h"
#include "SceneView.h"
#include "SceneTree.h"
#include "Inspector.h"
#include "Console.h"

namespace RexEditor
{
	static void OnEngineStarted()
	{
		// Needs the RenderApi to be initialized
		PanelManager::RegisterPanel<SceneViewPanel>("Scene View");
	}

	RE_STATIC_CONSTRUCTOR({

		PanelManager::RegisterPanel<NewProjectPanel>(); // Dont make a menubar item because the order is in PanelManager.cpp/Init
		PanelManager::RegisterPanel<SceneTreePanel>("Scene Tree");
		PanelManager::RegisterPanel<FileExplorerPanel>("File Explorer");
		PanelManager::RegisterPanel<InspectorPanel>("Inspector");
		PanelManager::RegisterPanel<ConsolePanel>("Console");

		EditorEvents::OnEditorStarted().Register<&OnEngineStarted>();
	});
}