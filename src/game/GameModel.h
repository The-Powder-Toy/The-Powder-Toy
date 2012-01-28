#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include "search/Save.h"
#include "simulation/Simulation.h"
#include "Renderer.h"
#include "GameView.h"
#include "Brush.h"
#include "client/User.h"

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
	int currentBrush;
	vector<Brush *> brushList;
	Save * currentSave;
	Simulation * sim;
	Renderer * ren;
	Tool * activeTool;
	User currentUser;
	//bool zoomEnabled;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
	void notifySaveChanged();
	void notifyBrushChanged();
	void notifyMenuListChanged();
	void notifyToolListChanged();
	void notifyActiveToolChanged();
	void notifyUserChanged();
	void notifyZoomChanged();
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
	User GetUser();
	void SetUser(User user);
	void SetBrush(int i);
	int GetBrushID();
	Simulation * GetSimulation();
	Renderer * GetRenderer();
	void SetZoomEnabled(bool enabled);
	bool GetZoomEnabled();
	void SetZoomSize(int size);
	int GetZoomSize();
	void SetZoomFactor(int factor);
	int GetZoomFactor();
	void SetZoomPosition(ui::Point position);
	ui::Point GetZoomPosition();
	void SetZoomWindowPosition(ui::Point position);
	ui::Point GetZoomWindowPosition();
};

#endif // GAMEMODEL_H
