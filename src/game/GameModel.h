#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include "search/Save.h"
#include "simulation/Simulation.h"
#include "Renderer.h"
#include "GameView.h"
#include "Brush.h"

#include "Tool.h"
#include "Menu.h"

using namespace std;

class GameView;
class Simulation;
class Renderer;

class GameModel
{
private:
	vector<GameView*> observers;
	vector<Tool*> toolList;
	vector<Menu*> menuList;
	Menu * activeMenu;
	Brush * currentBrush;
	Save * currentSave;
	Simulation * sim;
	Renderer * ren;
	Tool * activeTool;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
	void notifySaveChanged();
	void notifyBrushChanged();
	void notifyMenuListChanged();
	void notifyToolListChanged();
public:
	GameModel();
	~GameModel();
	Save * GetSave();
	Brush * GetBrush();
	void SetSave(Save * newSave);
	void AddObserver(GameView * observer);
	Tool * GetActiveTool();
	void SetActiveTool(Tool * tool);
	bool GetPaused();
	void SetPaused(bool pauseState);
	void ClearSimulation();
	vector<Menu*> GetMenuList();
	vector<Tool*> GetToolList();
	void SetActiveMenu(Menu * menu);
	Menu * GetActiveMenu();

	Simulation * GetSimulation();
	Renderer * GetRenderer();
};

#endif // GAMEMODEL_H
