
#include <iostream>
#include <queue>
#include "Config.h"
#include "GameController.h"
#include "GameModel.h"
#include "search/SearchController.h"
#include "interface/Point.h"

using namespace std;

GameController::GameController():
		search(NULL)
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
		ui::Engine::Ref().CloseWindow();
		delete search;
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

void GameController::Tick()
{
	gameModel->GetSimulation()->update_particles();
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
	//TODO: Implement
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
	//TODO: Implement
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


