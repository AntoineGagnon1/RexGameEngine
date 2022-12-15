#include "REDPch.h"
#include <RexEngine.h>

#include "PanelManager.h"

#include "NewProject.h"
#include "FileExplorer.h"
#include "SceneView.h"
#include "SceneTree.h"
#include "Inspector.h"

namespace RexEditor
{
	RE_STATIC_CONSTRUCTOR({

		PanelManager::RegisterPanel<NewProjectPanel>(); // Dont make a menubar item because the order is in PanelManager.cpp/Init
		PanelManager::RegisterPanel<SceneTreePanel>("Scene Tree");
		PanelManager::RegisterPanel<FileExplorerPanel>("File Explorer");
		PanelManager::RegisterPanel<InspectorPanel>("Inspector");
	});
}