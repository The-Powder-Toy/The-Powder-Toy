#include "interface/Engine.h"
#include "GameModel.h"
#include "GameView.h"
#include "simulation/Simulation.h"
#include "Renderer.h"
#include "interface/Point.h"
#include "Brush.h"

GameModel::GameModel():
	activeTool(NULL),
	sim(NULL),
	ren(NULL),
	currentSave(NULL),
	currentBrush(new Brush(ui::Point(4, 4)))
{
	sim = new Simulation();
	ren = new Renderer(ui::Engine::Ref().g, sim);

	menuList.clear();
	for(int i = 0; i < 12; i++)
	{
		menuList.push_back(new Menu((const char)sim->msections[i].icon[0], sim->msections[i].name));
	}
	//Build menus from Simulation elements
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->ptypes[i].menusection < 12 && sim->ptypes[i].enabled && sim->ptypes[i].menu)
		{
			Tool * tempTool = new ElementTool(i, sim->ptypes[i].name, PIXR(sim->ptypes[i].pcolors), PIXG(sim->ptypes[i].pcolors), PIXB(sim->ptypes[i].pcolors));
			menuList[sim->ptypes[i].menusection]->AddTool(tempTool);
		}
	}
	//Build other menus from wall data
	for(int i = 0; i < UI_WALLCOUNT; i++)
	{
		Tool * tempTool = new ElementTool(i+UI_WALLSTART, "", PIXR(sim->wtypes[i].colour), PIXG(sim->wtypes[i].colour), PIXB(sim->wtypes[i].colour));
		menuList[SC_WALL]->AddTool(tempTool);
		//sim->wtypes[i]
	}

	activeTool = new ElementTool(1, "TURD", 0, 0, 0);
}

GameModel::~GameModel()
{
	for(int i = 0; i < menuList.size(); i++)
	{
		for(int j = 0; i < menuList[i]->GetToolList().size(); i++)
		{
			delete menuList[i]->GetToolList()[j];
		}
		delete menuList[i];
	}
	delete sim;
	delete ren;
}

Brush * GameModel::GetBrush()
{
	return currentBrush;
}

void GameModel::AddObserver(GameView * observer){
	observers.push_back(observer);

	observer->NotifySimulationChanged(this);
	observer->NotifyRendererChanged(this);
	observer->NotifyPausedChanged(this);
	observer->NotifySaveChanged(this);
	observer->NotifyBrushChanged(this);
	observer->NotifyMenuListChanged(this);
	observer->NotifyToolListChanged(this);
}

void GameModel::SetActiveMenu(Menu * menu)
{
	for(int i = 0; i < menuList.size(); i++)
	{
		if(menuList[i]==menu)
		{
			activeMenu = menu;
			toolList = menu->GetToolList();
			notifyToolListChanged();
		}
	}
}

vector<Tool*> GameModel::GetToolList()
{
	return toolList;
}

Menu * GameModel::GetActiveMenu()
{
	return activeMenu;
}

Tool * GameModel::GetActiveTool()
{
	return activeTool;
}

void GameModel::SetActiveTool(Tool * tool)
{
	activeTool = tool;
	notifyActiveToolChanged();
}

vector<Menu*> GameModel::GetMenuList()
{
	return menuList;
}

Save * GameModel::GetSave()
{
	return currentSave;
}

void GameModel::SetSave(Save * newSave)
{
	currentSave = newSave;
	notifySaveChanged();
}

Simulation * GameModel::GetSimulation()
{
	return sim;
}

Renderer * GameModel::GetRenderer()
{
	return ren;
}

void GameModel::SetPaused(bool pauseState)
{
	sim->sys_pause = pauseState?1:0;
	notifyPausedChanged();
}

bool GameModel::GetPaused()
{
	return sim->sys_pause?true:false;
}

void GameModel::ClearSimulation()
{
	sim->clear_sim();
}

void GameModel::notifyRendererChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

void GameModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void GameModel::notifySimulationChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySimulationChanged(this);
	}
}

void GameModel::notifyPausedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyBrushChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyBrushChanged(this);
	}
}

void GameModel::notifyMenuListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyMenuListChanged(this);
	}
}

void GameModel::notifyToolListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyToolListChanged(this);
	}
}

void GameModel::notifyActiveToolChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyActiveToolChanged(this);
	}
}
