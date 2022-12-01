#include "REDPch.h"
#include <RexEngine.h>

#include "PanelManager.h"

#include "NewProject.h"
#include "FileExplorer.h"
#include "SceneView.h"

namespace RexEditor
{
	RE_STATIC_CONSTRUCTOR({

		PanelManager::RegisterPanel<NewProjectPanel>(); // Dont make a menubar item because the order is in PanelManager.cpp/Init
		PanelManager::RegisterPanel<FileExplorerPanel>("File Explorer");
	});
}