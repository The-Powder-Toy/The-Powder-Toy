#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <queue>
#include "GameView.h"
#include "GameModel.h"
#include "interface/Point.h"
#include "simulation/Simulation.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "login/LoginController.h"
#include "Menu.h"

using namespace std;

class GameModel;
class GameView;
class GameController
{
private:
	//Simulation * sim;
	GameView * gameView;
	GameModel * gameModel;
	SearchController * search;
	RenderController * renderOptions;
	LoginController * loginWindow;
public:
	class LoginCallback;
	GameController();
	~GameController();
	GameView * GetView();
	void AdjustBrushSize(int direction);
	void DrawPoints(queue<ui::Point*> & pointQueue);
	void Update();
	void SetPaused(bool pauseState);
	void SetPaused();
	void SetActiveMenu(Menu * menu);
	void SetActiveTool(Tool * tool);
	void OpenSearch();
	void OpenLogin();
	void OpenTags();
	void OpenDisplayOptions();
	void OpenRenderOptions();
	void OpenSaveWindow();
	void ClearSim();
	void ReloadSim();
	void Vote(int direction);
};

#endif // GAMECONTROLLER_H
