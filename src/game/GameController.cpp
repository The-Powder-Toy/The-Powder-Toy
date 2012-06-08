
#include <iostream>
#include <queue>
#include "Config.h"
#include "GameController.h"
#include "GameModel.h"
#include "client/SaveInfo.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "login/LoginController.h"
#include "interface/Point.h"
#include "dialogues/ErrorMessage.h"
#include "GameModelException.h"

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
			try
			{
				cc->gameModel->SetSave(new SaveInfo(*(cc->search->GetLoadedSave())));
			}
			catch(GameModelException & ex)
			{
				new ErrorMessage("Cannot open save", ex.what());
			}
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

class GameController::OptionsCallback: public ControllerCallback
{
	GameController * cc;
public:
	OptionsCallback(GameController * cc_) { cc = cc_; }
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
			cc->gameModel->SetSave(new SaveInfo(*(cc->ssave->GetSave())));

		}
		//cc->gameModel->SetUser(cc->loginWindow->GetUser());
	}
};

class GameController::TagsCallback: public ControllerCallback
{
	GameController * cc;
public:
	TagsCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		cc->gameModel->SetSave(new SaveInfo(*(cc->tagsWindow->GetSave())));
	}
};

class GameController::StampsCallback: public ControllerCallback
{
	GameController * cc;
public:
	StampsCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->localBrowser->GetSave())
		{
			cc->gameModel->SetStamp(cc->localBrowser->GetSave()->GetGameSave());
		}
		else
			cc->gameModel->SetStamp(NULL);
	}
};

GameController::GameController():
		search(NULL),
		renderOptions(NULL),
		loginWindow(NULL),
		ssave(NULL),
		console(NULL),
		tagsWindow(NULL),
		options(NULL)
{
	gameView = new GameView();
	gameModel = new GameModel();

	gameView->AttachController(this);
	gameModel->AddObserver(gameView);

	commandInterface = new LuaScriptInterface(gameModel);//new TPTScriptInterface();
	//commandInterface->AttachGameModel(gameModel);

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
	if(tagsWindow)
	{
		delete tagsWindow;
	}
	if(console)
	{
		delete console;
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

void GameController::PlaceStamp(ui::Point position)
{
	if(gameModel->GetStamp())
	{
		gameModel->GetSimulation()->Load(position.X, position.Y, gameModel->GetStamp());
		gameModel->SetPaused(gameModel->GetPaused());
	}
}

void GameController::PlaceClipboard(ui::Point position)
{
	if(gameModel->GetClipboard())
	{
		gameModel->GetSimulation()->Load(position.X, position.Y, gameModel->GetClipboard());
		gameModel->SetPaused(gameModel->GetPaused());
	}
}

void GameController::AdjustBrushSize(int direction, bool logarithmic)
{
	ui::Point newSize(0, 0);
	if(logarithmic)
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(direction * ((gameModel->GetBrush()->GetRadius().X/10)>0?gameModel->GetBrush()->GetRadius().X/10:1), direction * ((gameModel->GetBrush()->GetRadius().Y/10)>0?gameModel->GetBrush()->GetRadius().Y/10:1));
	else
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(direction, direction);
	if(newSize.X<0)
			newSize.X = 0;
	if(newSize.Y<0)
			newSize.Y = 0;
	gameModel->GetBrush()->SetRadius(newSize);
}

void GameController::AdjustZoomSize(int direction, bool logarithmic)
{
	int newSize;
	if(logarithmic)
		newSize = gameModel->GetZoomSize()+direction;
	else
		newSize = gameModel->GetZoomSize()+(((gameModel->GetZoomSize()/10)>0?(gameModel->GetZoomSize()/10):1)*direction);
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

ui::Point GameController::NormaliseBlockCoord(ui::Point point)
{
	return (point/CELL)*CELL;
}

void GameController::DrawRect(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->DrawRect(sim, cBrush, PointTranslate(point1), PointTranslate(point2));
}

void GameController::DrawLine(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->DrawLine(sim, cBrush, PointTranslate(point1), PointTranslate(point2));
}

void GameController::DrawFill(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->DrawFill(sim, cBrush, PointTranslate(point));
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

void GameController::ToolClick(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->Click(sim, cBrush, PointTranslate(point));
}

void GameController::StampRegion(ui::Point point1, ui::Point point2)
{
	GameSave * newSave;
	newSave = gameModel->GetSimulation()->Save(point1.X, point1.Y, point2.X, point2.Y);
	if(newSave)
		gameModel->AddStamp(newSave);
	else
		new ErrorMessage("Could not create stamp", "Error generating save file");
}

void GameController::CopyRegion(ui::Point point1, ui::Point point2)
{
	GameSave * newSave;
	newSave = gameModel->GetSimulation()->Save(point1.X, point1.Y, point2.X, point2.Y);
	if(newSave)
		gameModel->SetClipboard(newSave);
}

bool GameController::MouseMove(int x, int y, int dx, int dy)
{
	return commandInterface->OnMouseMove(x, y, dx, dy);
}

bool GameController::MouseDown(int x, int y, unsigned button)
{
	return commandInterface->OnMouseDown(x, y, button);
}

bool GameController::MouseUp(int x, int y, unsigned button)
{
	return commandInterface->OnMouseUp(x, y, button);
}

bool GameController::MouseWheel(int x, int y, int d)
{
	return commandInterface->OnMouseWheel(x, y, d);
}

bool GameController::KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	return commandInterface->OnKeyPress(key, character, shift, ctrl, alt);
}

bool GameController::KeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	return commandInterface->OnKeyRelease(key, character, shift, ctrl, alt);
}

void GameController::Tick()
{
	commandInterface->OnTick();
}

void GameController::Update()
{
	ui::Point pos = gameView->GetMousePosition();
	if(pos.X >= 0 && pos.Y >= 0 && pos.X < XRES && pos.Y < YRES)
	{
		gameView->SetSample(gameModel->GetSimulation()->Get(pos.X, pos.Y));
	}

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

void GameController::SetDecoration(bool decorationState)
{
	gameModel->SetDecoration(decorationState);
}

void GameController::SetDecoration()
{
	gameModel->SetDecoration(!gameModel->GetDecoration());
}

void GameController::SetColour(ui::Colour colour)
{
	gameModel->SetColourSelectorColour(colour);
}

void GameController::SetActiveMenu(Menu * menu)
{
	gameModel->SetActiveMenu(menu);
	vector<Menu*> menuList = gameModel->GetMenuList();
	bool set = false;
	for(int i = 0; i < menuList.size(); i++)
	{
		if(menuList[i]==menu && i == SC_DECO)
		{
			gameModel->SetColourSelectorVisibility(true);
			set = true;
		}
	}
	if(!set)
		gameModel->SetColourSelectorVisibility(false);
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
	if(gameModel->GetUser().ID)
	{
		if(gameModel->GetSave() && gameModel->GetSave()->GetID())
		{
			tagsWindow = new TagsController(new TagsCallback(this), gameModel->GetSave());
			ui::Engine::Ref().ShowWindow(tagsWindow->GetView());
		}
		else
		{
			new ErrorMessage("Error", "No save open");
		}
	}
	else
	{
		new ErrorMessage("Error", "You need to login to edit tags.");
	}
}

void GameController::OpenStamps()
{
	localBrowser = new LocalBrowserController(new StampsCallback(this));
	ui::Engine::Ref().ShowWindow(localBrowser->GetView());
}

void GameController::OpenOptions()
{
	options = new OptionsController(gameModel->GetSimulation(), new OptionsCallback(this));
	ui::Engine::Ref().ShowWindow(options->GetView());

}

void GameController::ShowConsole()
{
	if(!console)
		console = new ConsoleController(NULL, commandInterface);
	ui::Engine::Ref().ShowWindow(console->GetView());
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
		GameSave * gameSave = gameModel->GetSimulation()->Save();
		if(!gameSave)
		{
			new ErrorMessage("Error", "Unable to build save.");
		}
		else
		{
			if(gameModel->GetSave())
			{
				SaveInfo tempSave(*gameModel->GetSave());
				tempSave.SetGameSave(gameSave);
				ssave = new SSaveController(new SSaveCallback(this), tempSave);
			}
			else
			{				
				SaveInfo tempSave(0, 0, 0, 0, gameModel->GetUser().Username, "");
				tempSave.SetGameSave(gameSave);
				ssave = new SSaveController(new SSaveCallback(this), tempSave);
			}
			ui::Engine::Ref().ShowWindow(ssave->GetView());
		}
	}
	else
	{
		new ErrorMessage("Error", "You need to login to upload saves.");
	}
}

void GameController::FrameStep()
{
	gameModel->FrameStep(1);
	gameModel->SetPaused(true);
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
	if(gameModel->GetSave() && gameModel->GetSave()->GetGameSave())
	{
		gameModel->GetSimulation()->Load(gameModel->GetSave()->GetGameSave());
	}
}

std::string GameController::ElementResolve(int type)
{
	if(gameModel && gameModel->GetSimulation() && gameModel->GetSimulation()->elements && type >= 0 && type < PT_NUM)
		return std::string(gameModel->GetSimulation()->elements[type].Name);
	else
		return "";
}

