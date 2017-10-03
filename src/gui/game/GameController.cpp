#include <iostream>
#include <queue>
#include "Config.h"
#include "Format.h"
#include "Platform.h"
#include "GameController.h"
#include "GameModel.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "gui/search/SearchController.h"
#include "gui/render/RenderController.h"
#include "gui/login/LoginController.h"
#include "gui/interface/Point.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "GameModelException.h"
#include "simulation/Air.h"
#include "gui/elementsearch/ElementSearchActivity.h"
#include "gui/profile/ProfileActivity.h"
#include "gui/colourpicker/ColourPickerActivity.h"
#include "gui/update/UpdateActivity.h"
#include "Notification.h"
#include "gui/filebrowser/FileBrowserActivity.h"
#include "gui/save/LocalSaveActivity.h"
#include "gui/save/ServerSaveActivity.h"
#include "gui/interface/Keys.h"
#include "gui/interface/Mouse.h"
#include "simulation/Snapshot.h"
#include "debug/DebugInfo.h"
#include "debug/DebugParts.h"
#include "debug/ElementPopulation.h"
#include "debug/DebugLines.h"
#include "debug/ParticleDebug.h"
#ifdef LUACONSOLE
#include "lua/LuaScriptInterface.h"
#else
#include "lua/TPTScriptInterface.h"
#endif

using namespace std;

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
				cc->HistorySnapshot();
				cc->gameModel->SetSave(cc->search->GetLoadedSave());
				cc->search->ReleaseLoadedSave();
			}
			catch(GameModelException & ex)
			{
				new ErrorMessage("Cannot open save", ex.what());
			}
		}
	}
};

class GameController::SaveOpenCallback: public ControllerCallback
{
	GameController * cc;
public:
	SaveOpenCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->activePreview->GetDoOpen() && cc->activePreview->GetSaveInfo())
		{
			try
			{
				cc->HistorySnapshot();
				cc->LoadSave(cc->activePreview->GetSaveInfo());
			}
			catch(GameModelException & ex)
			{
				new ErrorMessage("Cannot open save", ex.what());
			}
		}
	}
};

class GameController::OptionsCallback: public ControllerCallback
{
	GameController * cc;
public:
	OptionsCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		cc->gameModel->UpdateQuickOptions();
	}
};

class GameController::TagsCallback: public ControllerCallback
{
	GameController * cc;
public:
	TagsCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		cc->gameView->NotifySaveChanged(cc->gameModel);
	}
};

class GameController::StampsCallback: public ControllerCallback
{
	GameController * cc;
public:
	StampsCallback(GameController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		SaveFile *file = cc->localBrowser->GetSave();
		if (file)
		{
			if (file->GetError().length())
				new ErrorMessage("Error loading stamp", file->GetError());
			else if (cc->localBrowser->GetMoveToFront())
				Client::Ref().MoveStampToFront(file->GetName());
			cc->LoadStamp(file->GetGameSave());
		}
	}
};

GameController::GameController():
	firstTick(true),
	foundSignID(-1),
	activePreview(NULL),
	search(NULL),
	renderOptions(NULL),
	loginWindow(NULL),
	console(NULL),
	tagsWindow(NULL),
	localBrowser(NULL),
	options(NULL),
	debugFlags(0),
	HasDone(false)
{
	gameView = new GameView();
	gameModel = new GameModel();
	gameModel->BuildQuickOptionMenu(this);

	gameView->AttachController(this);
	gameModel->AddObserver(gameView);

	gameView->SetDebugHUD(Client::Ref().GetPrefBool("Renderer.DebugMode", false));

#ifdef LUACONSOLE
	commandInterface = new LuaScriptInterface(this, gameModel);
	((LuaScriptInterface*)commandInterface)->SetWindow(gameView);
#else
	commandInterface = new TPTScriptInterface(this, gameModel);
#endif

	ActiveToolChanged(0, gameModel->GetActiveTool(0));
	ActiveToolChanged(1, gameModel->GetActiveTool(1));
	ActiveToolChanged(2, gameModel->GetActiveTool(2));
	ActiveToolChanged(3, gameModel->GetActiveTool(3));

	Client::Ref().AddListener(this);

	debugInfo.push_back(new DebugParts(0x1, gameModel->GetSimulation()));
	debugInfo.push_back(new ElementPopulationDebug(0x2, gameModel->GetSimulation()));
	debugInfo.push_back(new DebugLines(0x4, gameView, this));
	debugInfo.push_back(new ParticleDebug(0x8, gameModel->GetSimulation(), gameModel));
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
	if(activePreview)
	{
		delete activePreview;
	}
	if(localBrowser)
	{
		delete localBrowser;
	}
	if (options)
	{
		delete options;
	}
	for(std::vector<DebugInfo*>::iterator iter = debugInfo.begin(), end = debugInfo.end(); iter != end; iter++)
	{
		delete *iter;
	}
	//deleted here because it refuses to be deleted when deleted from gameModel even with the same code
	std::deque<Snapshot*> history = gameModel->GetHistory();
	for(std::deque<Snapshot*>::iterator iter = history.begin(), end = history.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	std::vector<QuickOption*> quickOptions = gameModel->GetQuickOptions();
	for(std::vector<QuickOption*>::iterator iter = quickOptions.begin(), end = quickOptions.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	std::vector<Notification*> notifications = gameModel->GetNotifications();
	for(std::vector<Notification*>::iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
	{
		delete *iter;
	}
	delete gameModel;
	if (gameView->CloseActiveWindow())
	{
		delete gameView;
	}
}

void GameController::HistoryRestore()
{
	std::deque<Snapshot*> history = gameModel->GetHistory();
	if (!history.size())
		return;
	unsigned int historyPosition = gameModel->GetHistoryPosition();
	unsigned int newHistoryPosition = std::max((int)historyPosition-1, 0);
	// When undoing, save the current state as a final redo
	// This way ctrl+y will always bring you back to the point right before your last ctrl+z
	if (historyPosition == history.size())
	{
		Snapshot * newSnap = gameModel->GetSimulation()->CreateSnapshot();
		if (newSnap)
			newSnap->Authors = Client::Ref().GetAuthorInfo();
		delete gameModel->GetRedoHistory();
		gameModel->SetRedoHistory(newSnap);
	}
	Snapshot * snap = history[newHistoryPosition];
	gameModel->GetSimulation()->Restore(*snap);
	Client::Ref().OverwriteAuthorInfo(snap->Authors);
	gameModel->SetHistory(history);
	gameModel->SetHistoryPosition(newHistoryPosition);
}

void GameController::HistorySnapshot()
{
	std::deque<Snapshot*> history = gameModel->GetHistory();
	unsigned int historyPosition = gameModel->GetHistoryPosition();
	Snapshot * newSnap = gameModel->GetSimulation()->CreateSnapshot();
	if (newSnap)
	{
		newSnap->Authors = Client::Ref().GetAuthorInfo();
		while (historyPosition < history.size())
		{
			Snapshot * snap = history.back();
			history.pop_back();
			delete snap;
		}
		if (history.size() >= gameModel->GetUndoHistoryLimit())
		{
			Snapshot * snap = history.front();
			history.pop_front();
			delete snap;
			if (historyPosition > history.size())
				historyPosition--;
		}
		history.push_back(newSnap);
		gameModel->SetHistory(history);
		gameModel->SetHistoryPosition(std::min((size_t)historyPosition+1, history.size()));
		delete gameModel->GetRedoHistory();
		gameModel->SetRedoHistory(NULL);
	}
}

void GameController::HistoryForward()
{
	std::deque<Snapshot*> history = gameModel->GetHistory();
	if (!history.size())
		return;
	unsigned int historyPosition = gameModel->GetHistoryPosition();
	unsigned int newHistoryPosition = std::min((size_t)historyPosition+1, history.size());
	Snapshot *snap;
	if (newHistoryPosition == history.size())
		snap = gameModel->GetRedoHistory();
	else
		snap = history[newHistoryPosition];
	if (!snap)
		return;
	gameModel->GetSimulation()->Restore(*snap);
	Client::Ref().OverwriteAuthorInfo(snap->Authors);
	gameModel->SetHistoryPosition(newHistoryPosition);
}

GameView * GameController::GetView()
{
	return gameView;
}

int GameController::GetSignAt(int x, int y)
{
	Simulation * sim = gameModel->GetSimulation();
	for (int i = sim->signs.size()-1; i >= 0; i--)
	{
		int signx, signy, signw, signh;
		sim->signs[i].pos(sim->signs[i].getText(sim), signx, signy, signw, signh);
		if (x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
			return i;
	}
	return -1;
}

// assumed to already be a valid sign
std::string GameController::GetSignText(int signID)
{
	return gameModel->GetSimulation()->signs[signID].text;
}

void GameController::PlaceSave(ui::Point position, bool includePressure)
{
	GameSave *placeSave = gameModel->GetPlaceSave();
	if (placeSave)
	{
		HistorySnapshot();
		if (!gameModel->GetSimulation()->Load(position.X, position.Y, placeSave, includePressure))
		{
			gameModel->SetPaused(placeSave->paused | gameModel->GetPaused());
			Client::Ref().MergeStampAuthorInfo(placeSave->authors);
		}
	}
}

void GameController::Install()
{
#if defined(MACOSX)
	new InformationMessage("No installation necessary", "You don't need to install The Powder Toy on OS X", false);
#elif defined(WIN) || defined(LIN)
	class InstallConfirmation: public ConfirmDialogueCallback {
	public:
		GameController * c;
		InstallConfirmation(GameController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
				if(Client::Ref().DoInstallation())
				{
					new InformationMessage("Success", "Installation completed", false);
				}
				else
				{
					new ErrorMessage("Could not install", "The installation did not complete due to an error");
				}
			}
		}
		virtual ~InstallConfirmation() { }
	};
	new ConfirmPrompt("Install The Powder Toy", "Do you wish to install The Powder Toy on this computer?\nThis allows you to open save files and saves directly from the website.", new InstallConfirmation(this));
#else
	new ErrorMessage("Cannot install", "You cannot install The Powder Toy on this platform");
#endif
}

void GameController::AdjustGridSize(int direction)
{
	if(direction > 0)
		gameModel->GetRenderer()->SetGridSize((gameModel->GetRenderer()->GetGridSize()+1)%10);
	else
		gameModel->GetRenderer()->SetGridSize((gameModel->GetRenderer()->GetGridSize()+9)%10);
}

void GameController::InvertAirSim()
{
	gameModel->GetSimulation()->air->Invert();
}


void GameController::AdjustBrushSize(int direction, bool logarithmic, bool xAxis, bool yAxis)
{
	if(xAxis && yAxis)
		return;

	ui::Point newSize(0, 0);
	ui::Point oldSize = gameModel->GetBrush()->GetRadius();
	if(logarithmic)
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(direction * ((gameModel->GetBrush()->GetRadius().X/5)>0?gameModel->GetBrush()->GetRadius().X/5:1), direction * ((gameModel->GetBrush()->GetRadius().Y/5)>0?gameModel->GetBrush()->GetRadius().Y/5:1));
	else
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(direction, direction);
	if(newSize.X < 0)
		newSize.X = 0;
	if(newSize.Y < 0)
		newSize.Y = 0;
	if(newSize.X > 200)
		newSize.X = 200;
	if(newSize.Y > 200)
		newSize.Y = 200;

	if(xAxis)
		gameModel->GetBrush()->SetRadius(ui::Point(newSize.X, oldSize.Y));
	else if(yAxis)
		gameModel->GetBrush()->SetRadius(ui::Point(oldSize.X, newSize.Y));
	else
		gameModel->GetBrush()->SetRadius(newSize);
}

void GameController::SetBrushSize(ui::Point newSize)
{
	gameModel->GetBrush()->SetRadius(newSize);
}

void GameController::AdjustZoomSize(int direction, bool logarithmic)
{
	int newSize;
	if(logarithmic)
		newSize = gameModel->GetZoomSize()+(((gameModel->GetZoomSize()/10)>0?(gameModel->GetZoomSize()/10):1)*direction);
	else
		newSize = gameModel->GetZoomSize()+direction;
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

bool GameController::MouseInZoom(ui::Point position)
{
	if(position.X >= XRES)
		position.X = XRES-1;
	if(position.Y >= YRES)
		position.Y = YRES-1;
	if(position.Y < 0)
		position.Y = 0;
	if(position.X < 0)
		position.X = 0;

	return gameModel->MouseInZoom(position);
}

ui::Point GameController::PointTranslate(ui::Point point)
{
	if(point.X >= XRES)
		point.X = XRES-1;
	if(point.Y >= YRES)
		point.Y = YRES-1;
	if(point.Y < 0)
		point.Y = 0;
	if(point.X < 0)
		point.X = 0;

	return gameModel->AdjustZoomCoords(point);
}

ui::Point GameController::NormaliseBlockCoord(ui::Point point)
{
	return (point/CELL)*CELL;
}

void GameController::DrawRect(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->SetStrength(1.0f);
	activeTool->DrawRect(sim, cBrush, point1, point2);
}

void GameController::DrawLine(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->SetStrength(1.0f);
	activeTool->DrawLine(sim, cBrush, point1, point2);
}

void GameController::DrawFill(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->SetStrength(1.0f);
	activeTool->DrawFill(sim, cBrush, point);
}

void GameController::DrawPoints(int toolSelection, ui::Point oldPos, ui::Point newPos, bool held)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush * cBrush = gameModel->GetBrush();
	if (!activeTool || !cBrush)
	{
		return;
	}

	activeTool->SetStrength(gameModel->GetToolStrength());
	if (!held)
		activeTool->Draw(sim, cBrush, newPos);
	else
		activeTool->DrawLine(sim, cBrush, oldPos, newPos, true);
}

bool GameController::LoadClipboard()
{
	GameSave *clip = gameModel->GetClipboard();
	if (!clip)
		return false;
	gameModel->SetPlaceSave(clip);
	if (gameModel->GetPlaceSave() && gameModel->GetPlaceSave()->Collapsed())
		gameModel->GetPlaceSave()->Expand();
	return true;
}

void GameController::LoadStamp(GameSave *stamp)
{
	gameModel->SetPlaceSave(stamp);
	if(gameModel->GetPlaceSave() && gameModel->GetPlaceSave()->Collapsed())
		gameModel->GetPlaceSave()->Expand();
}

void GameController::TranslateSave(ui::Point point)
{
	vector2d translate = v2d_new(point.X, point.Y);
	vector2d translated = gameModel->GetPlaceSave()->Translate(translate);
	ui::Point currentPlaceSaveOffset = gameView->GetPlaceSaveOffset();
	// resets placeSaveOffset to 0, which is why we back it up first
	gameModel->SetPlaceSave(gameModel->GetPlaceSave());
	gameView->SetPlaceSaveOffset(ui::Point(translated.x, translated.y) + currentPlaceSaveOffset);
}

void GameController::TransformSave(matrix2d transform)
{
	vector2d translate = v2d_zero;
	gameModel->GetPlaceSave()->Transform(transform, translate);
	gameModel->SetPlaceSave(gameModel->GetPlaceSave());
}

void GameController::ToolClick(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush * cBrush = gameModel->GetBrush();
	if(!activeTool || !cBrush)
		return;
	activeTool->Click(sim, cBrush, point);
}

std::string GameController::StampRegion(ui::Point point1, ui::Point point2, bool includePressure)
{
	GameSave * newSave;
	newSave = gameModel->GetSimulation()->Save(point1.X, point1.Y, point2.X, point2.Y, includePressure);
	if(newSave)
	{
		newSave->paused = gameModel->GetPaused();
		std::string stampName = Client::Ref().AddStamp(newSave);
		if (stampName.length() == 0)
			new ErrorMessage("Could not create stamp", "Error serializing save file");
		return stampName;
	}
	else
	{
		new ErrorMessage("Could not create stamp", "Error generating save file");
		return "";
	}
}

void GameController::CopyRegion(ui::Point point1, ui::Point point2, bool includePressure)
{
	GameSave * newSave;
	newSave = gameModel->GetSimulation()->Save(point1.X, point1.Y, point2.X, point2.Y, includePressure);
	if(newSave)
	{
		Json::Value clipboardInfo;
		clipboardInfo["type"] = "clipboard";
		clipboardInfo["username"] = Client::Ref().GetAuthUser().Username;
		clipboardInfo["date"] = (Json::Value::UInt64)time(NULL);
		Client::Ref().SaveAuthorInfo(&clipboardInfo);
		newSave->authors = clipboardInfo;

		newSave->paused = gameModel->GetPaused();
		gameModel->SetClipboard(newSave);
	}
}

void GameController::CutRegion(ui::Point point1, ui::Point point2, bool includePressure)
{
	CopyRegion(point1, point2, includePressure);
	gameModel->GetSimulation()->clear_area(point1.X, point1.Y, point2.X-point1.X, point2.Y-point1.Y);
}

bool GameController::MouseMove(int x, int y, int dx, int dy)
{
	return commandInterface->OnMouseMove(x, y, dx, dy);
}

bool GameController::MouseDown(int x, int y, unsigned button)
{
	bool ret = commandInterface->OnMouseDown(x, y, button);
	if (ret && y<YRES && x<XRES && !gameView->GetPlacingSave() && !gameView->GetPlacingZoom())
	{
		ui::Point point = gameModel->AdjustZoomCoords(ui::Point(x, y));
		x = point.X;
		y = point.Y;
		if (!gameModel->GetActiveTool(0) || gameModel->GetActiveTool(0)->GetIdentifier() != "DEFAULT_UI_SIGN" || button != SDL_BUTTON_LEFT) //If it's not a sign tool or you are right/middle clicking
		{
			foundSignID = GetSignAt(x, y);
			if (foundSignID != -1)
			{
				sign foundSign = gameModel->GetSimulation()->signs[foundSignID];
				if (sign::splitsign(foundSign.text.c_str()))
					return false;
			}
		}
	}
	return ret;
}

bool GameController::MouseUp(int x, int y, unsigned button, char type)
{
	bool ret = commandInterface->OnMouseUp(x, y, button, type);
	if (type)
		return ret;
	if (ret && foundSignID != -1 && y<YRES && x<XRES && !gameView->GetPlacingSave())
	{
		ui::Point point = gameModel->AdjustZoomCoords(ui::Point(x, y));
		x = point.X;
		y = point.Y;
		if (!gameModel->GetActiveTool(0) || gameModel->GetActiveTool(0)->GetIdentifier() != "DEFAULT_UI_SIGN" || button != SDL_BUTTON_LEFT) //If it's not a sign tool or you are right/middle clicking
		{
			int foundSignID = GetSignAt(x, y);
			if (foundSignID != -1)
			{
				sign foundSign = gameModel->GetSimulation()->signs[foundSignID];
				const char* str = foundSign.text.c_str();
				char type;
				int pos = sign::splitsign(str, &type);
				if (pos)
				{
					ret = false;
					if (type == 'c' || type == 't' || type == 's')
					{
						char buff[256];
						strcpy(buff, str+3);
						buff[pos-3] = 0;
						switch (type)
						{
						case 'c':
						{
							int saveID = format::StringToNumber<int>(std::string(buff));
							if (saveID)
								OpenSavePreview(saveID, 0, false);
							break;
						}
						case 't':
						{
							// buff is already confirmed to be a number by sign::splitsign
							std::stringstream uri;
							uri << "http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=" << buff;
							Platform::OpenURI(uri.str());
							break;
						}
						case 's':
							OpenSearch(buff);
							break;
						}
					}
					else if (type == 'b')
					{
						Simulation * sim = gameModel->GetSimulation();
						sim->create_part(-1, foundSign.x, foundSign.y, PT_SPRK);
					}
				}
			}
		}
	}
	foundSignID = -1;
	return ret;
}

bool GameController::MouseWheel(int x, int y, int d)
{
	return commandInterface->OnMouseWheel(x, y, d);
}

bool GameController::KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	bool ret = commandInterface->OnKeyPress(key, character, shift, ctrl, alt);
	if (ret)
	{
		Simulation * sim = gameModel->GetSimulation();
		if (!gameView->GetPlacingSave())
		{
			// Go right command
			if (key == SDLK_RIGHT)
			{
				sim->player.comm = (int)(sim->player.comm)|0x02;
			}
			// Go left command
			if (key == SDLK_LEFT)
			{
				sim->player.comm = (int)(sim->player.comm)|0x01;
			}
			// Use element command
			if (key == SDLK_DOWN && ((int)(sim->player.comm)&0x08)!=0x08)
			{
				sim->player.comm = (int)(sim->player.comm)|0x08;
			}
			// Jump command
			if (key == SDLK_UP && ((int)(sim->player.comm)&0x04)!=0x04)
			{
				sim->player.comm = (int)(sim->player.comm)|0x04;
			}
		}

		// Go right command
		if (key == SDLK_d)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x02;
		}
		// Go left command
		if (key == SDLK_a)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x01;
		}
		// Use element command
		if (key == SDLK_s && ((int)(sim->player2.comm)&0x08)!=0x08)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x08;
		}
		// Jump command
		if (key == SDLK_w && ((int)(sim->player2.comm)&0x04)!=0x04)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x04;
		}

		if (!sim->elementCount[PT_STKM2] || ctrl)
		{
			switch(key)
			{
			case 'w':
				SwitchGravity();
				break;
			case 'd':
				gameView->SetDebugHUD(!gameView->GetDebugHUD());
				break;
			case 's':
				gameView->BeginStampSelection();
				break;
			}
		}

		for(std::vector<DebugInfo*>::iterator iter = debugInfo.begin(), end = debugInfo.end(); iter != end; iter++)
		{
			if ((*iter)->ID & debugFlags)
				if (!(*iter)->KeyPress(key, character, shift, ctrl, alt, gameView->GetMousePosition()))
					ret = false;
		}
	}
	return ret;
}

bool GameController::KeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	bool ret = commandInterface->OnKeyRelease(key, character, shift, ctrl, alt);
	if (ret)
	{
		Simulation * sim = gameModel->GetSimulation();
		if (key == SDLK_RIGHT || key == SDLK_LEFT)
		{
			sim->player.pcomm = sim->player.comm;  //Saving last movement
			sim->player.comm = (int)(sim->player.comm)&12;  //Stop command
		}
		if (key == SDLK_UP)
		{
			sim->player.comm = (int)(sim->player.comm)&11;
		}
		if (key == SDLK_DOWN)
		{
			sim->player.comm = (int)(sim->player.comm)&7;
		}

		if (key == SDLK_d || key == SDLK_a)
		{
			sim->player2.pcomm = sim->player2.comm;  //Saving last movement
			sim->player2.comm = (int)(sim->player2.comm)&12;  //Stop command
		}
		if (key == SDLK_w)
		{
			sim->player2.comm = (int)(sim->player2.comm)&11;
		}
		if (key == SDLK_s)
		{
			sim->player2.comm = (int)(sim->player2.comm)&7;
		}
	}
	return ret;
}

bool GameController::MouseTick()
{
	return commandInterface->OnMouseTick();
}

void GameController::Tick()
{
	if(firstTick)
	{
#ifdef LUACONSOLE
		((LuaScriptInterface*)commandInterface)->Init();
#endif
#if !defined(MACOSX) && !defined(NO_INSTALL_CHECK)
		if (Client::Ref().IsFirstRun())
		{
			Install();
		}
#endif
		firstTick = false;
	}
	for(std::vector<DebugInfo*>::iterator iter = debugInfo.begin(), end = debugInfo.end(); iter != end; iter++)
	{
		if ((*iter)->ID & debugFlags)
			(*iter)->Draw();
	}
	commandInterface->OnTick();
}

void GameController::Exit()
{
	gameView->CloseActiveWindow();
	HasDone = true;
}

void GameController::ResetAir()
{
	Simulation * sim = gameModel->GetSimulation();
	sim->air->Clear();
	for (int i = 0; i < NPART; i++)
	{
		if (sim->parts[i].type == PT_QRTZ || sim->parts[i].type == PT_GLAS || sim->parts[i].type == PT_TUNG)
		{
			sim->parts[i].pavg[0] = sim->parts[i].pavg[1] = 0;
		}
	}
}

void GameController::ResetSpark()
{
	Simulation * sim = gameModel->GetSimulation();
	for (int i = 0; i < NPART; i++)
		if (sim->parts[i].type == PT_SPRK)
		{
			if (sim->parts[i].ctype >= 0 && sim->parts[i].ctype < PT_NUM && sim->elements[sim->parts[i].ctype].Enabled)
			{
				sim->parts[i].type = sim->parts[i].ctype;
				sim->parts[i].ctype = sim->parts[i].life = 0;
			}
			else
				sim->kill_part(i);
		}
	memset(sim->wireless, 0, sizeof(sim->wireless));
}

void GameController::SwitchGravity()
{
	gameModel->GetSimulation()->gravityMode = (gameModel->GetSimulation()->gravityMode+1)%3;

	switch (gameModel->GetSimulation()->gravityMode)
	{
	case 0:
		gameModel->SetInfoTip("Gravity: Vertical");
		break;
	case 1:
		gameModel->SetInfoTip("Gravity: Off");
		break;
	case 2:
		gameModel->SetInfoTip("Gravity: Radial");
		break;
	}
}

void GameController::SwitchAir()
{
	gameModel->GetSimulation()->air->airMode = (gameModel->GetSimulation()->air->airMode+1)%5;

	switch (gameModel->GetSimulation()->air->airMode)
	{
	case 0:
		gameModel->SetInfoTip("Air: On");
		break;
	case 1:
		gameModel->SetInfoTip("Air: Pressure Off");
		break;
	case 2:
		gameModel->SetInfoTip("Air: Velocity Off");
		break;
	case 3:
		gameModel->SetInfoTip("Air: Off");
		break;
	case 4:
		gameModel->SetInfoTip("Air: No Update");
		break;
	}
}

void GameController::ToggleAHeat()
{
	gameModel->SetAHeatEnable(!gameModel->GetAHeatEnable());
}

bool GameController::GetAHeatEnable()
{
	return gameModel->GetAHeatEnable();
}

void GameController::ToggleNewtonianGravity()
{
	if (gameModel->GetSimulation()->grav->ngrav_enable)
		gameModel->GetSimulation()->grav->stop_grav_async();
	else
		gameModel->GetSimulation()->grav->start_grav_async();
	gameModel->UpdateQuickOptions();
}


void GameController::LoadRenderPreset(int presetNum)
{
	Renderer * renderer = gameModel->GetRenderer();
	RenderPreset preset = renderer->renderModePresets[presetNum];
	gameModel->SetInfoTip(preset.Name);
	renderer->SetRenderMode(preset.RenderModes);
	renderer->SetDisplayMode(preset.DisplayModes);
	renderer->SetColourMode(preset.ColourMode);
}

void GameController::Update()
{
	ui::Point pos = gameView->GetMousePosition();
	gameModel->GetRenderer()->mousePos = PointTranslate(pos);
	if (pos.X < XRES && pos.Y < YRES)
		gameView->SetSample(gameModel->GetSimulation()->GetSample(PointTranslate(pos).X, PointTranslate(pos).Y));
	else
		gameView->SetSample(gameModel->GetSimulation()->GetSample(pos.X, pos.Y));

	Simulation * sim = gameModel->GetSimulation();
	sim->BeforeSim();
	if (!sim->sys_pause || sim->framerender)
	{
		sim->UpdateParticles(0, NPART);
		sim->AfterSim();
	}

	//if either STKM or STK2 isn't out, reset it's selected element. Defaults to PT_DUST unless right selected is something else
	//This won't run if the stickmen dies in a frame, since it respawns instantly
	if (!sim->player.spwn || !sim->player2.spwn)
	{
		int rightSelected = PT_DUST;
		Tool * activeTool = gameModel->GetActiveTool(1);
		if (activeTool->GetIdentifier().find("DEFAULT_PT_") != activeTool->GetIdentifier().npos)
		{
			int sr = activeTool->GetToolID();
			if (sr && sim->IsValidElement(sr))
				rightSelected = sr;
		}

		if (!sim->player.spwn)
			Element_STKM::STKM_set_element(sim, &sim->player, rightSelected);
		if (!sim->player2.spwn)
			Element_STKM::STKM_set_element(sim, &sim->player2, rightSelected);
	}
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

	if(activePreview && activePreview->HasExited)
	{
		delete activePreview;
		activePreview = NULL;
	}

	if(loginWindow && loginWindow->HasExited)
	{
		delete loginWindow;
		loginWindow = NULL;
	}

	if(localBrowser && localBrowser->HasDone)
	{
		delete localBrowser;
		localBrowser = NULL;
	}
}

void GameController::SetZoomEnabled(bool zoomEnabled)
{
	gameModel->SetZoomEnabled(zoomEnabled);
}

void GameController::SetToolStrength(float value)
{
	gameModel->SetToolStrength(value);
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

void GameController::ShowGravityGrid()
{
	gameModel->ShowGravityGrid(!gameModel->GetGravityGrid());
	gameModel->UpdateQuickOptions();
}

void GameController::SetHudEnable(bool hudState)
{
	gameView->SetHudEnable(hudState);
}

bool GameController::GetHudEnable()
{
	return gameView->GetHudEnable();
}

void GameController::SetDebugHUD(bool hudState)
{
	gameView->SetDebugHUD(hudState);
}

bool GameController::GetDebugHUD()
{
	return gameView->GetDebugHUD();
}

void GameController::SetActiveColourPreset(int preset)
{
	gameModel->SetActiveColourPreset(preset);
}

void GameController::SetColour(ui::Colour colour)
{
	gameModel->SetColourSelectorColour(colour);
	gameModel->SetPresetColour(colour);
}

void GameController::SetActiveMenu(int menuID)
{
	gameModel->SetActiveMenu(menuID);
	if(menuID == SC_DECO)
		gameModel->SetColourSelectorVisibility(true);
	else
		gameModel->SetColourSelectorVisibility(false);
}

std::vector<Menu*> GameController::GetMenuList()
{
	return gameModel->GetMenuList();
}

int GameController::GetNumMenus(bool onlyEnabled)
{
	int count = 0;
	if (onlyEnabled)
	{
		std::vector<Menu*> menuList = gameModel->GetMenuList();
		for (std::vector<Menu*>::iterator it = menuList.begin(), end = menuList.end(); it != end; ++it)
			if ((*it)->GetVisible())
				count++;
	}
	else
		count = gameModel->GetMenuList().size();
	return count;
}

void GameController::RebuildFavoritesMenu()
{
	gameModel->BuildFavoritesMenu();
}

void GameController::ActiveToolChanged(int toolSelection, Tool *tool)
{
	commandInterface->OnActiveToolChanged(toolSelection, tool);
}

Tool * GameController::GetActiveTool(int selection)
{
	return gameModel->GetActiveTool(selection);
}

void GameController::SetActiveTool(int toolSelection, Tool * tool)
{
	if (gameModel->GetActiveMenu() == SC_DECO && toolSelection == 2)
		toolSelection = 0;
	gameModel->SetActiveTool(toolSelection, tool);
	gameModel->GetRenderer()->gravityZonesEnabled = false;
	if (toolSelection == 3)
		gameModel->GetSimulation()->replaceModeSelected = tool->GetToolID();
	gameModel->SetLastTool(tool);
	for(int i = 0; i < 3; i++)
	{
		if(gameModel->GetActiveTool(i) == gameModel->GetMenuList().at(SC_WALL)->GetToolList().at(WL_GRAV))
			gameModel->GetRenderer()->gravityZonesEnabled = true;
	}
	if(tool->GetIdentifier() == "DEFAULT_UI_PROPERTY")
		((PropertyTool *)tool)->OpenWindow(gameModel->GetSimulation());
}

void GameController::SetLastTool(Tool * tool)
{
	gameModel->SetLastTool(tool);
}

int GameController::GetReplaceModeFlags()
{
	return gameModel->GetSimulation()->replaceModeFlags;
}

void GameController::SetReplaceModeFlags(int flags)
{
	gameModel->GetSimulation()->replaceModeFlags = flags;
}

void GameController::OpenSearch(std::string searchText)
{
	if(!search)
		search = new SearchController(new SearchCallback(this));
	if (searchText.length())
		search->DoSearch2(searchText);
	ui::Engine::Ref().ShowWindow(search->GetView());
}

void GameController::OpenLocalSaveWindow(bool asCurrent)
{
	Simulation * sim = gameModel->GetSimulation();
	GameSave * gameSave = sim->Save();
	if(!gameSave)
	{
		new ErrorMessage("Error", "Unable to build save.");
	}
	else
	{
		gameSave->paused = gameModel->GetPaused();

		SaveFile tempSave("");
		if (gameModel->GetSaveFile())
		{
			tempSave.SetFileName(gameModel->GetSaveFile()->GetName());
			tempSave.SetDisplayName(gameModel->GetSaveFile()->GetDisplayName());
		}
		tempSave.SetGameSave(gameSave);

		if (!asCurrent || !gameModel->GetSaveFile())
		{
			class LocalSaveCallback: public FileSavedCallback
			{
				GameController * c;
			public:
				LocalSaveCallback(GameController * _c): c(_c) {}
				virtual  ~LocalSaveCallback() {}
				virtual void FileSaved(SaveFile* file)
				{
					c->gameModel->SetSaveFile(file);
				}
			};

			new LocalSaveActivity(tempSave, new LocalSaveCallback(this));
		}
		else if (gameModel->GetSaveFile())
		{
			Json::Value localSaveInfo;
			localSaveInfo["type"] = "localsave";
			localSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
			localSaveInfo["title"] = gameModel->GetSaveFile()->GetName();
			localSaveInfo["date"] = (Json::Value::UInt64)time(NULL);
			Client::Ref().SaveAuthorInfo(&localSaveInfo);
			gameSave->authors = localSaveInfo;
			
			gameModel->SetSaveFile(&tempSave);
			Client::Ref().MakeDirectory(LOCAL_SAVE_DIR);
			std::vector<char> saveData = gameSave->Serialise();
			if (saveData.size() == 0)
				new ErrorMessage("Error", "Unable to serialize game data.");
			else if (Client::Ref().WriteFile(gameSave->Serialise(), gameModel->GetSaveFile()->GetName()))
				new ErrorMessage("Error", "Unable to write save file.");
			else
				gameModel->SetInfoTip("Saved Successfully");
		}
	}
}

void GameController::LoadSaveFile(SaveFile * file)
{
	gameModel->SetSaveFile(file);
}


void GameController::LoadSave(SaveInfo * save)
{
	gameModel->SetSave(save);
}

void GameController::OpenSavePreview(int saveID, int saveDate, bool instant)
{
	activePreview = new PreviewController(saveID, saveDate, instant, new SaveOpenCallback(this));
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}

void GameController::OpenSavePreview()
{
	if(gameModel->GetSave())
	{
		activePreview = new PreviewController(gameModel->GetSave()->GetID(), false, new SaveOpenCallback(this));
		ui::Engine::Ref().ShowWindow(activePreview->GetView());
	}
}

void GameController::OpenLocalBrowse()
{
	class LocalSaveOpenCallback: public FileSelectedCallback
	{
		GameController * c;
	public:
		LocalSaveOpenCallback(GameController * _c): c(_c) {}
		virtual  ~LocalSaveOpenCallback() {};
		virtual void FileSelected(SaveFile* file)
		{
			c->HistorySnapshot();
			c->LoadSaveFile(file);
			delete file;
		}
	};
	new FileBrowserActivity(LOCAL_SAVE_DIR PATH_SEP, new LocalSaveOpenCallback(this));
}

void GameController::OpenLogin()
{
	loginWindow = new LoginController();
	ui::Engine::Ref().ShowWindow(loginWindow->GetView());
}

void GameController::OpenProfile()
{
	if(Client::Ref().GetAuthUser().ID)
	{
		new ProfileActivity(Client::Ref().GetAuthUser().Username);
	}
	else
	{
		loginWindow = new LoginController();
		ui::Engine::Ref().ShowWindow(loginWindow->GetView());
	}
}

void GameController::OpenElementSearch()
{
	vector<Tool*> toolList;
	vector<Menu*> menuList = gameModel->GetMenuList();
	for(std::vector<Menu*>::iterator iter = menuList.begin(), end = menuList.end(); iter!=end; ++iter) {
		if(!(*iter))
			continue;
		vector<Tool*> menuToolList = (*iter)->GetToolList();
		if(!menuToolList.size())
			continue;
		toolList.insert(toolList.end(), menuToolList.begin(), menuToolList.end());
	}
	vector<Tool*> hiddenTools = gameModel->GetUnlistedTools();
	toolList.insert(toolList.end(), hiddenTools.begin(), hiddenTools.end());
	new ElementSearchActivity(this, toolList);
}

void GameController::OpenColourPicker()
{
	class ColourPickerCallback: public ColourPickedCallback
	{
		GameController * c;
	public:
		ColourPickerCallback(GameController * _c): c(_c) {}
		virtual  ~ColourPickerCallback() {};
		virtual void ColourPicked(ui::Colour colour)
		{
			c->SetColour(colour);
		}
	};
	new ColourPickerActivity(gameModel->GetColourSelectorColour(), new ColourPickerCallback(this));
}

void GameController::OpenTags()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetID())
	{
		delete tagsWindow;
		tagsWindow = new TagsController(new TagsCallback(this), gameModel->GetSave());
		ui::Engine::Ref().ShowWindow(tagsWindow->GetView());
	}
	else
	{
		new ErrorMessage("Error", "No save open");
	}
}

void GameController::OpenStamps()
{
	localBrowser = new LocalBrowserController(new StampsCallback(this));
	ui::Engine::Ref().ShowWindow(localBrowser->GetView());
}

void GameController::OpenOptions()
{
	options = new OptionsController(gameModel, new OptionsCallback(this));
	ui::Engine::Ref().ShowWindow(options->GetView());

}

void GameController::ShowConsole()
{
	if (!console)
		console = new ConsoleController(NULL, commandInterface);
	if (console->GetView() != ui::Engine::Ref().GetWindow())
		ui::Engine::Ref().ShowWindow(console->GetView());
}

void GameController::HideConsole()
{
	if (!console)
		return;
	console->GetView()->CloseActiveWindow();
}

void GameController::OpenRenderOptions()
{
	renderOptions = new RenderController(gameModel->GetRenderer(), NULL);
	ui::Engine::Ref().ShowWindow(renderOptions->GetView());
}

void GameController::OpenSaveWindow()
{
	class SaveUploadedCallback: public ServerSaveActivity::SaveUploadedCallback
	{
		GameController * c;
	public:
		SaveUploadedCallback(GameController * _c): c(_c) {}
		virtual  ~SaveUploadedCallback() {}
		virtual void SaveUploaded(SaveInfo save)
		{
			save.SetVote(1);
			save.SetVotesUp(1);
			c->LoadSave(&save);
		}
	};
	if(gameModel->GetUser().ID)
	{
		Simulation * sim = gameModel->GetSimulation();
		GameSave * gameSave = sim->Save();
		if(!gameSave)
		{
			new ErrorMessage("Error", "Unable to build save.");
		}
		else
		{
			gameSave->paused = gameModel->GetPaused();

			if(gameModel->GetSave())
			{
				SaveInfo tempSave(*gameModel->GetSave());
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, new SaveUploadedCallback(this));
			}
			else
			{
				SaveInfo tempSave(0, 0, 0, 0, 0, gameModel->GetUser().Username, "");
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, new SaveUploadedCallback(this));
			}
		}
	}
	else
	{
		new ErrorMessage("Error", "You need to login to upload saves.");
	}
}

void GameController::SaveAsCurrent()
{

	class SaveUploadedCallback: public ServerSaveActivity::SaveUploadedCallback
	{
		GameController * c;
	public:
		SaveUploadedCallback(GameController * _c): c(_c) {}
		virtual  ~SaveUploadedCallback() {}
		virtual void SaveUploaded(SaveInfo save)
		{
			c->LoadSave(&save);
		}
	};

	if(gameModel->GetSave() && gameModel->GetUser().ID && gameModel->GetUser().Username == gameModel->GetSave()->GetUserName())
	{
		Simulation * sim = gameModel->GetSimulation();
		GameSave * gameSave = sim->Save();
		if(!gameSave)
		{
			new ErrorMessage("Error", "Unable to build save.");
		}
		else
		{
			gameSave->paused = gameModel->GetPaused();

			if(gameModel->GetSave())
			{
				SaveInfo tempSave(*gameModel->GetSave());
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, true, new SaveUploadedCallback(this));
			}
			else
			{
				SaveInfo tempSave(0, 0, 0, 0, 0, gameModel->GetUser().Username, "");
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, true, new SaveUploadedCallback(this));
			}
		}
	}
	else if(gameModel->GetUser().ID)
	{
		OpenSaveWindow();
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
	{
		try
		{
			gameModel->SetVote(direction);
		}
		catch(GameModelException & ex)
		{
			new ErrorMessage("Error while voting", ex.what());
		}
	}
}

void GameController::ChangeBrush()
{
	gameModel->SetBrushID(gameModel->GetBrushID()+1);
}

void GameController::ClearSim()
{
	HistorySnapshot();
	gameModel->SetSave(NULL);
	gameModel->ClearSimulation();
}

void GameController::ReloadSim()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSave(gameModel->GetSave());
	}
	else if(gameModel->GetSaveFile() && gameModel->GetSaveFile()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSaveFile(gameModel->GetSaveFile());
	}
}

std::string GameController::ElementResolve(int type, int ctype)
{
	if(gameModel && gameModel->GetSimulation())
	{
		if (type == PT_LIFE && ctype >= 0 && ctype < NGOL)
			return gameModel->GetSimulation()->gmenu[ctype].name;
		else if (type >= 0 && type < PT_NUM)
			return std::string(gameModel->GetSimulation()->elements[type].Name);
	}
	return "";
}

bool GameController::IsValidElement(int type)
{
	if(gameModel && gameModel->GetSimulation())
	{
		return (type && gameModel->GetSimulation()->IsValidElement(type));
	}
	else
		return false;
}

std::string GameController::WallName(int type)
{
	if(gameModel && gameModel->GetSimulation() && type >= 0 && type < UI_WALLCOUNT)
		return std::string(gameModel->GetSimulation()->wtypes[type].name);
	else
		return "";
}

int GameController::Record(bool record)
{
	return gameView->Record(record);
}

void GameController::NotifyAuthUserChanged(Client * sender)
{
	User newUser = sender->GetAuthUser();
	gameModel->SetUser(newUser);
}

void GameController::NotifyNewNotification(Client * sender, std::pair<std::string, std::string> notification)
{
	class LinkNotification : public Notification
	{
		std::string link;
	public:
		LinkNotification(std::string link_, std::string message) : Notification(message), link(link_) {}
		virtual ~LinkNotification() {}

		virtual void Action()
		{
			Platform::OpenURI(link);
		}
	};
	gameModel->AddNotification(new LinkNotification(notification.second, notification.first));
}

void GameController::NotifyUpdateAvailable(Client * sender)
{
	class UpdateConfirmation: public ConfirmDialogueCallback {
	public:
		GameController * c;
		UpdateConfirmation(GameController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
				c->RunUpdater();
			}
		}
		virtual ~UpdateConfirmation() { }
	};

	class UpdateNotification : public Notification
	{
		GameController * c;
	public:
		UpdateNotification(GameController * c, std::string message) : Notification(message), c(c) {}
		virtual ~UpdateNotification() {}

		virtual void Action()
		{
			UpdateInfo info = Client::Ref().GetUpdateInfo();
			std::stringstream updateMessage;
			updateMessage << "Are you sure you want to run the updater? Please save any changes before updating.\n\nCurrent version:\n ";

#ifdef SNAPSHOT
			updateMessage << "Snapshot " << SNAPSHOT_ID;
#elif MOD_ID > 0
			updateMessage << "Mod version " << SNAPSHOT_ID;
#elif defined(BETA)
			updateMessage << SAVE_VERSION << "." << MINOR_VERSION << " Beta, Build " << BUILD_NUM;
#else
			updateMessage << SAVE_VERSION << "." << MINOR_VERSION << " Stable, Build " << BUILD_NUM;
#endif

			updateMessage << "\nNew version:\n ";
			if (info.Type == UpdateInfo::Beta)
				updateMessage << info.Major << " " << info.Minor << " Beta, Build " << info.Build;
			else if (info.Type == UpdateInfo::Snapshot)
#if MOD_ID > 0
				updateMessage << "Mod version " << info.Time;
#else
				updateMessage << "Snapshot " << info.Time;
#endif
			else if(info.Type == UpdateInfo::Stable)
				updateMessage << info.Major << " " << info.Minor << " Stable, Build " << info.Build;

			if (info.Changelog.length())
				updateMessage << "\n\nChangelog:\n" << info.Changelog;

			new ConfirmPrompt("Run Updater", updateMessage.str(), new UpdateConfirmation(c));
		}
	};

	switch(sender->GetUpdateInfo().Type)
	{
		case UpdateInfo::Snapshot:
#if MOD_ID > 0
			gameModel->AddNotification(new UpdateNotification(this, std::string("A new mod update is available - click here to update")));
#else
			gameModel->AddNotification(new UpdateNotification(this, std::string("A new snapshot is available - click here to update")));
#endif
			break;
		case UpdateInfo::Stable:
			gameModel->AddNotification(new UpdateNotification(this, std::string("A new version is available - click here to update")));
			break;
		case UpdateInfo::Beta:
			gameModel->AddNotification(new UpdateNotification(this, std::string("A new beta is available - click here to update")));
			break;
	}
}

void GameController::RemoveNotification(Notification * notification)
{
	gameModel->RemoveNotification(notification);
}

void GameController::RunUpdater()
{
	Exit();
	new UpdateActivity();
}
