
#include <iostream>
#include <queue>
#include "Config.h"
#include "GameController.h"
#include "GameModel.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "login/LoginController.h"
#include "interface/Point.h"

using namespace std;

class GameController::LoginCallback: public ControllerCallback
{
	GameController * cc;
public:
	LoginCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		cc->gameModel->SetUser(cc->loginWindow->GetUser());
	}
};

GameController::GameController():
		search(NULL),
		renderOptions(NULL),
		loginWindow(NULL)
{
	gameView = new GameView();
	gameModel = new GameModel();

	gameView->AttachController(this);
	gameModel->AddObserver(gameView);

	//sim = new Simulation();
}

GameController::~GameController()
{
	if(search)
	{
		if(ui::Engine::Ref().GetWindow() == search->GetView())
			ui::Engine::Ref().CloseWindow();
		delete search;
	}
	if(renderOptions)
	{
		if(ui::Engine::Ref().GetWindow() == renderOptions->GetView())
			ui::Engine::Ref().CloseWindow();
		delete renderOptions;
	}
	if(loginWindow)
	{
		if(ui::Engine::Ref().GetWindow() == loginWindow->GetView())
			ui::Engine::Ref().CloseWindow();
		delete loginWindow;
	}
	delete gameView;
	delete gameModel;
}

GameView * GameController::GetView()
{
	return gameView;
}

void GameController::AdjustBrushSize(int direction)
{
	ui::Point newSize = gameModel->GetBrush()->GetRadius() + ui::Point(direction, direction);
	if(newSize.X<0)
			newSize.X = 0;
	if(newSize.Y<0)
			newSize.Y = 0;
	gameModel->GetBrush()->SetRadius(newSize);
}

void GameController::DrawPoints(queue<ui::Point*> & pointQueue)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool();
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
	{
		if(!pointQueue.empty())
		{
			while(!pointQueue.empty())
			{
				delete pointQueue.front();
				pointQueue.pop();
			}
		}
	}
	if(!pointQueue.empty())
	{
		ui::Point * sPoint = NULL;
		while(!pointQueue.empty())
		{
			ui::Point * fPoint = pointQueue.front();
			pointQueue.pop();
			if(sPoint)
			{
				activeTool->DrawLine(sim, cBrush, *fPoint, *sPoint);
				delete sPoint;
			}
			else
			{
				activeTool->Draw(sim, cBrush, *fPoint);
			}
			sPoint = fPoint;
		}
		if(sPoint)
			delete sPoint;
	}
}

void GameController::Update()
{
	//gameModel->GetSimulation()->update_particles();
	if(loginWindow && loginWindow->HasExited)
	{
		delete loginWindow;
		loginWindow = NULL;
	}
}

void GameController::SetPaused(bool pauseState)
{
	gameModel->SetPaused(pauseState);
}

void GameController::SetPaused()
{
	gameModel->SetPaused(!gameModel->GetPaused());
}

void GameController::SetActiveMenu(Menu * menu)
{
	gameModel->SetActiveMenu(menu);
}

void GameController::SetActiveTool(Tool * tool)
{
	gameModel->SetActiveTool(tool);
}

void GameController::OpenSearch()
{
	search = new SearchController();
	ui::Engine::Ref().ShowWindow(search->GetView());
}

void GameController::OpenLogin()
{
	loginWindow = new LoginController(new LoginCallback(this));
	ui::Engine::Ref().ShowWindow(loginWindow->GetView());
}

void GameController::OpenTags()
{
	//TODO: Implement
}

void GameController::OpenDisplayOptions()
{
	//TODO: Implement
}

void GameController::OpenRenderOptions()
{
	renderOptions = new RenderController(gameModel->GetRenderer());
	ui::Engine::Ref().ShowWindow(renderOptions->GetView());
}

void GameController::OpenSaveWindow()
{
	//TODO: Implement
}

void GameController::Vote(int direction)
{
	//TODO: Implement
}


void GameController::ClearSim()
{
	gameModel->ClearSimulation();
}

void GameController::ReloadSim()
{
	//TODO: Implement
}


