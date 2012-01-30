
#include <iostream>
#include <queue>
#include "Config.h"
#include "GameController.h"
#include "GameModel.h"
#include "search/Save.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "login/LoginController.h"
#include "interface/Point.h"
#include "dialogues/ErrorMessage.h"

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


class GameController::SearchCallback: public ControllerCallback
{
	GameController * cc;
public:
	SearchCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->search->GetLoadedSave())
		{
			cc->gameModel->SetSave(new Save(*(cc->search->GetLoadedSave())));
		}
	}
};


class GameController::RenderCallback: public ControllerCallback
{
	GameController * cc;
public:
	RenderCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		//cc->gameModel->SetUser(cc->loginWindow->GetUser());
	}
};

class GameController::SSaveCallback: public ControllerCallback
{
	GameController * cc;
public:
	SSaveCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->ssave->GetSaveUploaded())
		{
			cc->gameModel->SetSave(new Save(*(cc->ssave->GetSave())));
		}
		//cc->gameModel->SetUser(cc->loginWindow->GetUser());
	}
};

GameController::GameController():
		search(NULL),
		renderOptions(NULL),
		loginWindow(NULL),
		ssave(NULL)
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
		delete search;
	}
	if(renderOptions)
	{
		delete renderOptions;
	}
	if(loginWindow)
	{
		delete loginWindow;
	}
	if(ui::Engine::Ref().GetWindow() == gameView)
	{
		ui::Engine::Ref().CloseWindow();
	}
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

void GameController::AdjustZoomSize(int direction)
{
	int newSize = gameModel->GetZoomSize()+direction;
	if(newSize<5)
			newSize = 5;
	if(newSize>64)
			newSize = 64;
	gameModel->SetZoomSize(newSize);

	int newZoomFactor = 256/newSize;
	if(newZoomFactor<3)
		newZoomFactor = 3;
	gameModel->SetZoomFactor(newZoomFactor);
}

ui::Point GameController::PointTranslate(ui::Point point)
{
	bool zoomEnabled = gameModel->GetZoomEnabled();
	if(!zoomEnabled)
		return point;
	//If we try to draw inside the zoom window, normalise the coordinates
	int zoomFactor = gameModel->GetZoomFactor();
	ui::Point zoomWindowPosition = gameModel->GetZoomWindowPosition();
	ui::Point zoomWindowSize = ui::Point(gameModel->GetZoomSize()*zoomFactor, gameModel->GetZoomSize()*zoomFactor);

	if(point.X > zoomWindowPosition.X && point.X > zoomWindowPosition.Y && point.X < zoomWindowPosition.X+zoomWindowSize.X && point.Y < zoomWindowPosition.Y+zoomWindowSize.Y)
		return ((point-zoomWindowPosition)/gameModel->GetZoomFactor())+gameModel->GetZoomPosition();
	return point;
}

void GameController::DrawRect(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->DrawRect(sim, cBrush, point1, point2);
}

void GameController::DrawLine(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->DrawLine(sim, cBrush, point1, point2);
}

void GameController::DrawFill(int toolSelection, ui::Point point)
{

}

void GameController::DrawPoints(int toolSelection, queue<ui::Point*> & pointQueue)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
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
		ui::Point sPoint(0, 0);
		bool first = true;
		while(!pointQueue.empty())
		{
			ui::Point fPoint = PointTranslate(*pointQueue.front());
			delete pointQueue.front();
			pointQueue.pop();
			if(!first)
			{
				activeTool->DrawLine(sim, cBrush, fPoint, sPoint);
			}
			else
			{
				first = false;
				activeTool->Draw(sim, cBrush, fPoint);
			}
			sPoint = fPoint;
		}
	}
}

void GameController::Update()
{
	gameModel->GetSimulation()->update_particles();
	if(renderOptions && renderOptions->HasExited)
	{
		delete renderOptions;
		renderOptions = NULL;
	}

	if(search && search->HasExited)
	{
		delete search;
		search = NULL;
	}

	if(loginWindow && loginWindow->HasExited)
	{
		delete loginWindow;
		loginWindow = NULL;
	}
}

void GameController::SetZoomEnabled(bool zoomEnabled)
{
	gameModel->SetZoomEnabled(zoomEnabled);
}

void GameController::SetZoomPosition(ui::Point position)
{
	ui::Point zoomPosition = position-(gameModel->GetZoomSize()/2);
	if(zoomPosition.X < 0)
			zoomPosition.X = 0;
	if(zoomPosition.Y < 0)
			zoomPosition.Y = 0;
	if(zoomPosition.X >= XRES-gameModel->GetZoomSize())
			zoomPosition.X = XRES-gameModel->GetZoomSize();
	if(zoomPosition.Y >= YRES-gameModel->GetZoomSize())
			zoomPosition.Y = YRES-gameModel->GetZoomSize();

	ui::Point zoomWindowPosition = ui::Point(0, 0);
	if(position.X < XRES/2)
		zoomWindowPosition.X = XRES-(gameModel->GetZoomSize()*gameModel->GetZoomFactor());

	gameModel->SetZoomPosition(zoomPosition);
	gameModel->SetZoomWindowPosition(zoomWindowPosition);
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

void GameController::SetActiveTool(int toolSelection, Tool * tool)
{
	gameModel->SetActiveTool(toolSelection, tool);
}

void GameController::OpenSearch()
{
	search = new SearchController(new SearchCallback(this));
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
	renderOptions = new RenderController(gameModel->GetRenderer(), new RenderCallback(this));
	ui::Engine::Ref().ShowWindow(renderOptions->GetView());
}

void GameController::OpenSaveWindow()
{
	if(gameModel->GetUser().ID)
	{
		if(gameModel->GetSave())
		{
			Save tempSave(*gameModel->GetSave());
			int tempSaveLength;
			tempSave.SetData(gameModel->GetSimulation()->Save(tempSaveLength));
			ssave = new SSaveController(new SSaveCallback(this), tempSave);
		}
		else
		{
			Save tempSave(0, 0, 0, 0, gameModel->GetUser().Username, "");
			int tempSaveLength;
			tempSave.SetData(gameModel->GetSimulation()->Save(tempSaveLength));
			ssave = new SSaveController(new SSaveCallback(this), tempSave);
		}
		ui::Engine::Ref().ShowWindow(ssave->GetView());
	}
	else
	{
		new ErrorMessage("Error", "You need to login to upload saves.");
	}
}

void GameController::Vote(int direction)
{
	if(gameModel->GetSave() && gameModel->GetUser().ID && gameModel->GetSave()->GetID() && gameModel->GetSave()->GetVote()==0)
		gameModel->SetVote(direction);
}

void GameController::ChangeBrush()
{
	gameModel->SetBrush(gameModel->GetBrushID()+1);
}

void GameController::ClearSim()
{
	gameModel->SetSave(NULL);
	gameModel->ClearSimulation();
}

void GameController::ReloadSim()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetData())
		gameModel->GetSimulation()->Load(gameModel->GetSave()->GetData(), gameModel->GetSave()->GetDataLength());
}


