#include "GameController.h"

#include "Brush.h"
#include "Config.h"
#include "Controller.h"
#include "Format.h"
#include "GameModel.h"
#include "GameModelException.h"
#include "GameView.h"
#include "Menu.h"
#include "Notification.h"
#include "QuickOptions.h"
#include "RenderPreset.h"
#include "Tool.h"

#ifdef LUACONSOLE
# include "lua/LuaScriptInterface.h"
# include "lua/LuaEvents.h"
#else
# include "lua/TPTScriptInterface.h"
#endif

#include "client/Client.h"
#include "client/GameSave.h"
#include "common/Platform.h"
#include "debug/DebugInfo.h"
#include "debug/DebugLines.h"
#include "debug/DebugParts.h"
#include "debug/ElementPopulation.h"
#include "debug/ParticleDebug.h"
#include "graphics/Renderer.h"
#include "simulation/Air.h"
#include "simulation/ElementClasses.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"
#include "simulation/Snapshot.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Keys.h"
#include "gui/interface/Mouse.h"
#include "gui/interface/Engine.h"

#include "gui/colourpicker/ColourPickerActivity.h"
#include "gui/elementsearch/ElementSearchActivity.h"
#include "gui/filebrowser/FileBrowserActivity.h"
#include "gui/profile/ProfileActivity.h"
#include "gui/save/LocalSaveActivity.h"
#include "gui/save/ServerSaveActivity.h"
#include "gui/update/UpdateActivity.h"

#include "gui/console/ConsoleController.h"
#include "gui/console/ConsoleView.h"
#include "gui/localbrowser/LocalBrowserController.h"
#include "gui/localbrowser/LocalBrowserView.h"
#include "gui/login/LoginController.h"
#include "gui/login/LoginView.h"
#include "gui/options/OptionsController.h"
#include "gui/options/OptionsView.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewView.h"
#include "gui/render/RenderController.h"
#include "gui/render/RenderView.h"
#include "gui/search/SearchController.h"
#include "gui/search/SearchView.h"
#include "gui/tags/TagsController.h"
#include "gui/tags/TagsView.h"

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

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
#else
	commandInterface = new TPTScriptInterface(this, gameModel);
#endif

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
	delete commandInterface;
	delete gameModel;
	if (gameView->CloseActiveWindow())
	{
		delete gameView;
	}
}

bool GameController::HistoryRestore()
{
	if (!gameModel->HistoryCanRestore())
	{
		return false;
	}
	// * When undoing for the first time since the last call to HistorySnapshot, save the current state.
	//   Ctrl+Y needs this in order to bring you back to the point right before your last Ctrl+Z, because
	//   the last history entry is what this Ctrl+Z brings you back to, not the current state.
	if (!beforeRestore)
	{
		beforeRestore = gameModel->GetSimulation()->CreateSnapshot();
		beforeRestore->Authors = Client::Ref().GetAuthorInfo();
	}
	gameModel->HistoryRestore();
	auto &current = *gameModel->HistoryCurrent();
	gameModel->GetSimulation()->Restore(current);
	Client::Ref().OverwriteAuthorInfo(current.Authors);

	return true;
}

void GameController::HistorySnapshot()
{
	// * Calling HistorySnapshot means the user decided to use the current state and
	//   forfeit the option to go back to whatever they Ctrl+Z'd their way back from.
	beforeRestore.reset();
	gameModel->HistoryPush(gameModel->GetSimulation()->CreateSnapshot());
}

bool GameController::HistoryForward()
{
	if (!gameModel->HistoryCanForward())
	{
		return false;
	}
	gameModel->HistoryForward();
	// * If gameModel has nothing more to give, we've Ctrl+Y'd our way back to the original
	//   state; restore this instead, then get rid of it.
	auto &current = gameModel->HistoryCurrent() ? *gameModel->HistoryCurrent() : *beforeRestore;
	gameModel->GetSimulation()->Restore(current);
	Client::Ref().OverwriteAuthorInfo(current.Authors);
	if (&current == beforeRestore.get())
	{
		beforeRestore.reset();
	}

	return true;
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
		sim->signs[i].getDisplayText(sim, signx, signy, signw, signh);
		if (x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
			return i;
	}
	return -1;
}

// assumed to already be a valid sign
String GameController::GetSignText(int signID)
{
	return gameModel->GetSimulation()->signs[signID].text;
}

std::pair<int, sign::Type> GameController::GetSignSplit(int signID)
{
	return gameModel->GetSimulation()->signs[signID].split();
}

void GameController::PlaceSave(ui::Point position)
{
	GameSave *placeSave = gameModel->GetPlaceSave();
	if (placeSave)
	{
		HistorySnapshot();
		if (!gameModel->GetSimulation()->Load(placeSave, !gameView->ShiftBehaviour(), position.X, position.Y))
		{
			gameModel->SetPaused(placeSave->paused | gameModel->GetPaused());
			Client::Ref().MergeStampAuthorInfo(placeSave->authors);
		}
	}
}

void GameController::Install()
{
#if defined(MACOSX)
	new InformationMessage("No installation necessary", "You don't need to install " APPNAME " on OS X", false);
#elif defined(WIN) || defined(LIN)
	new ConfirmPrompt("Install " APPNAME, "Do you wish to install " APPNAME " on this computer?\nThis allows you to open save files and saves directly from the website.", { [] {
		if (Client::Ref().DoInstallation())
		{
			new InformationMessage("Success", "Installation completed", false);
		}
		else
		{
			new ErrorMessage("Could not install", "The installation did not complete due to an error");
		}
	} });
#else
	new ErrorMessage("Cannot install", "You cannot install " APPNAME " on this platform");
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


void GameController::AdjustBrushSize(int delta, bool logarithmic, bool xAxis, bool yAxis)
{
	if(xAxis && yAxis)
		return;

	ui::Point newSize(0, 0);
	ui::Point oldSize = gameModel->GetBrush()->GetRadius();
	if(logarithmic)
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(delta * std::max(gameModel->GetBrush()->GetRadius().X / 5, 1), delta * std::max(gameModel->GetBrush()->GetRadius().Y / 5, 1));
	else
		newSize = gameModel->GetBrush()->GetRadius() + ui::Point(delta, delta);
	if(newSize.X < 0)
		newSize.X = 0;
	if(newSize.Y < 0)
		newSize.Y = 0;
	if(newSize.X > 200)
		newSize.X = 200;
	if(newSize.Y > 200)
		newSize.Y = 200;

	if(xAxis)
		SetBrushSize(ui::Point(newSize.X, oldSize.Y));
	else if(yAxis)
		SetBrushSize(ui::Point(oldSize.X, newSize.Y));
	else
		SetBrushSize(newSize);
}

void GameController::SetBrushSize(ui::Point newSize)
{
	gameModel->GetBrush()->SetRadius(newSize);
}

void GameController::AdjustZoomSize(int delta, bool logarithmic)
{
	int newSize;
	if(logarithmic)
		newSize = gameModel->GetZoomSize() + std::max(gameModel->GetZoomSize() / 10, 1) * delta;
	else
		newSize = gameModel->GetZoomSize() + delta;
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
	return true;
}

void GameController::LoadStamp(GameSave *stamp)
{
	gameModel->SetPlaceSave(stamp);
}

void GameController::TranslateSave(ui::Point point)
{
	vector2d translate = v2d_new(float(point.X), float(point.Y));
	vector2d translated = gameModel->GetPlaceSave()->Translate(translate);
	ui::Point currentPlaceSaveOffset = gameView->GetPlaceSaveOffset();
	// resets placeSaveOffset to 0, which is why we back it up first
	gameModel->SetPlaceSave(gameModel->GetPlaceSave());
	gameView->SetPlaceSaveOffset(ui::Point(int(translated.x), int(translated.y)) + currentPlaceSaveOffset);
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

ByteString GameController::StampRegion(ui::Point point1, ui::Point point2)
{
	GameSave * newSave = gameModel->GetSimulation()->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), point1.X, point1.Y, point2.X, point2.Y);
	if(newSave)
	{
		newSave->paused = gameModel->GetPaused();
		ByteString stampName = Client::Ref().AddStamp(newSave);
		delete newSave;
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

void GameController::CopyRegion(ui::Point point1, ui::Point point2)
{
	GameSave * newSave = gameModel->GetSimulation()->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), point1.X, point1.Y, point2.X, point2.Y);
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

void GameController::CutRegion(ui::Point point1, ui::Point point2)
{
	CopyRegion(point1, point2);
	HistorySnapshot();
	gameModel->GetSimulation()->clear_area(point1.X, point1.Y, point2.X-point1.X, point2.Y-point1.Y);
}

bool GameController::MouseMove(int x, int y, int dx, int dy)
{
	MouseMoveEvent ev(x, y, dx, dy);
	return commandInterface->HandleEvent(LuaEvents::mousemove, &ev);
}

bool GameController::MouseDown(int x, int y, unsigned button)
{
	MouseDownEvent ev(x, y, button);
	bool ret = commandInterface->HandleEvent(LuaEvents::mousedown, &ev);
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
				if (gameModel->GetSimulation()->signs[foundSignID].split().first)
				{
					return false;
				}
			}
		}
	}
	return ret;
}

bool GameController::MouseUp(int x, int y, unsigned button, MouseupReason reason)
{
	MouseUpEvent ev(x, y, button, reason);
	bool ret = commandInterface->HandleEvent(LuaEvents::mouseup, &ev);
	if (reason != mouseUpNormal)
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
				sign &foundSign = gameModel->GetSimulation()->signs[foundSignID];
				String str = foundSign.text;
				auto si = gameModel->GetSimulation()->signs[foundSignID].split();
				if (si.first)
				{
					ret = false;
					switch (si.second)
					{
					case sign::Type::Save:
						{
							int saveID = str.Substr(3, si.first - 3).ToNumber<int>(true);
							if (saveID)
								OpenSavePreview(saveID, 0, false);
						}
						break;
					case sign::Type::Thread:
						Platform::OpenURI(ByteString::Build(SCHEME "powdertoy.co.uk/Discussions/Thread/View.html?Thread=", str.Substr(3, si.first - 3).ToUtf8()));
						break;
					case sign::Type::Search:
						OpenSearch(str.Substr(3, si.first - 3));
						break;
					case sign::Type::Button:
						gameModel->GetSimulation()->create_part(-1, foundSign.x, foundSign.y, PT_SPRK);
						break;
					default: break;
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
	MouseWheelEvent ev(x, y, d);
	return commandInterface->HandleEvent(LuaEvents::mousewheel, &ev);
}

bool GameController::TextInput(String text)
{
	TextInputEvent ev(text);
	return commandInterface->HandleEvent(LuaEvents::textinput, &ev);
}

bool GameController::TextEditing(String text)
{
	TextEditingEvent ev(text);
	return commandInterface->HandleEvent(LuaEvents::textediting, &ev);
}

bool GameController::KeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	KeyEvent ev(key, scan, repeat, shift, ctrl, alt);
	bool ret = commandInterface->HandleEvent(LuaEvents::keypress, &ev);
	if (repeat)
		return ret;
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
			else if (key == SDLK_LEFT)
			{
				sim->player.comm = (int)(sim->player.comm)|0x01;
			}
			// Use element command
			else if (key == SDLK_DOWN && ((int)(sim->player.comm)&0x08)!=0x08)
			{
				sim->player.comm = (int)(sim->player.comm)|0x08;
			}
			// Jump command
			else if (key == SDLK_UP && ((int)(sim->player.comm)&0x04)!=0x04)
			{
				sim->player.comm = (int)(sim->player.comm)|0x04;
			}
		}

		// Go right command
		if (scan == SDL_SCANCODE_D)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x02;
		}
		// Go left command
		else if (scan == SDL_SCANCODE_A)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x01;
		}
		// Use element command
		else if (scan == SDL_SCANCODE_S && ((int)(sim->player2.comm)&0x08)!=0x08)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x08;
		}
		// Jump command
		else if (scan == SDL_SCANCODE_W && ((int)(sim->player2.comm)&0x04)!=0x04)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x04;
		}

		if (!sim->elementCount[PT_STKM2] || ctrl)
		{
			switch(scan)
			{
			case SDL_SCANCODE_W:
				SwitchGravity();
				break;
			case SDL_SCANCODE_D:
				gameView->SetDebugHUD(!gameView->GetDebugHUD());
				break;
			case SDL_SCANCODE_S:
				gameView->BeginStampSelection();
				break;
			}
		}

		for(std::vector<DebugInfo*>::iterator iter = debugInfo.begin(), end = debugInfo.end(); iter != end; iter++)
		{
			if ((*iter)->debugID & debugFlags)
				if (!(*iter)->KeyPress(key, scan, shift, ctrl, alt, gameView->GetMousePosition()))
					ret = false;
		}
	}
	return ret;
}

bool GameController::KeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	KeyEvent ev(key, scan, repeat, shift, ctrl, alt);
	bool ret = commandInterface->HandleEvent(LuaEvents::keyrelease, &ev);
	if (repeat)
		return ret;
	if (ret)
	{
		Simulation * sim = gameModel->GetSimulation();
		if (key == SDLK_RIGHT || key == SDLK_LEFT)
		{
			sim->player.pcomm = sim->player.comm;  //Saving last movement
			sim->player.comm = (int)(sim->player.comm)&12;  //Stop command
		}
		else if (key == SDLK_UP)
		{
			sim->player.comm = (int)(sim->player.comm)&11;
		}
		else if (key == SDLK_DOWN)
		{
			sim->player.comm = (int)(sim->player.comm)&7;
		}

		if (scan == SDL_SCANCODE_D || scan == SDL_SCANCODE_A)
		{
			sim->player2.pcomm = sim->player2.comm;  //Saving last movement
			sim->player2.comm = (int)(sim->player2.comm)&12;  //Stop command
		}
		else if (scan == SDL_SCANCODE_W)
		{
			sim->player2.comm = (int)(sim->player2.comm)&11;
		}
		else if (scan == SDL_SCANCODE_S)
		{
			sim->player2.comm = (int)(sim->player2.comm)&7;
		}
	}
	return ret;
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
	if (gameModel->SelectNextIdentifier.length())
	{
		gameModel->BuildMenus();
		gameModel->SetActiveTool(gameModel->SelectNextTool, gameModel->GetToolFromIdentifier(gameModel->SelectNextIdentifier));
		gameModel->SelectNextIdentifier.clear();
	}
	for(std::vector<DebugInfo*>::iterator iter = debugInfo.begin(), end = debugInfo.end(); iter != end; iter++)
	{
		if ((*iter)->debugID & debugFlags)
			(*iter)->Draw();
	}
	commandInterface->OnTick();
}

void GameController::Blur()
{
	// Tell lua that mouse is up (even if it really isn't)
	MouseUp(0, 0, 0, mouseUpBlur);
	BlurEvent ev;
	commandInterface->HandleEvent(LuaEvents::blur, &ev);
}

void GameController::Exit()
{
	CloseEvent ev;
	commandInterface->HandleEvent(LuaEvents::close, &ev);
	gameView->CloseActiveWindow();
	HasDone = true;
}

void GameController::ResetAir()
{
	Simulation * sim = gameModel->GetSimulation();
	sim->air->Clear();
	for (int i = 0; i < NPART; i++)
	{
		if (GameSave::PressureInTmp3(sim->parts[i].type))
		{
			sim->parts[i].tmp3 = 0;
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
	gameModel->GetSimulation()->gravityMode = (gameModel->GetSimulation()->gravityMode+1)%4;

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
	case 3:
		gameModel->SetInfoTip("Gravity: Custom");
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

void GameController::ResetAHeat()
{
	gameModel->ResetAHeat();
}

void GameController::ToggleNewtonianGravity()
{
	gameModel->SetNewtonianGravity(!gameModel->GetNewtonianGrvity());
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
		sim->UpdateParticles(0, NPART - 1);
		sim->AfterSim();
	}

	//if either STKM or STK2 isn't out, reset it's selected element. Defaults to PT_DUST unless right selected is something else
	//This won't run if the stickmen dies in a frame, since it respawns instantly
	if (!sim->player.spwn || !sim->player2.spwn)
	{
		int rightSelected = PT_DUST;
		Tool * activeTool = gameModel->GetActiveTool(1);
		if (activeTool->GetIdentifier().BeginsWith("DEFAULT_PT_"))
		{
			int sr = activeTool->GetToolID();
			if (sr && sim->IsElementOrNone(sr))
				rightSelected = sr;
		}

		void Element_STKM_set_element(Simulation *sim, playerst *playerp, int element);
		if (!sim->player.spwn)
			Element_STKM_set_element(sim, &sim->player, rightSelected);
		if (!sim->player2.spwn)
			Element_STKM_set_element(sim, &sim->player2, rightSelected);
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

void GameController::SetBrushEnable(bool brushState)
{
	gameView->SetBrushEnable(brushState);
}

bool GameController::GetBrushEnable()
{
	return gameView->GetBrushEnable();
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
	if(tool->GetIdentifier() == "DEFAULT_UI_ADDLIFE")
	{
		((GOLTool *)tool)->OpenWindow(gameModel->GetSimulation(), toolSelection);
	}
}

void GameController::SetActiveTool(int toolSelection, ByteString identifier)
{
	Tool *tool = gameModel->GetToolFromIdentifier(identifier);
	if (!tool)
		return;
	SetActiveTool(toolSelection, tool);
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
	int old_flags = gameModel->GetSimulation()->replaceModeFlags;
	if (!(old_flags & REPLACE_MODE) && (flags & REPLACE_MODE))
	{
		// if replace mode has just been enabled, disable specific delete
		flags &= ~SPECIFIC_DELETE;
	}
	if (!(old_flags & SPECIFIC_DELETE) && (flags & SPECIFIC_DELETE))
	{
		// if specific delete has just been enabled, disable replace mode
		flags &= ~REPLACE_MODE;
	}
	if ((flags & SPECIFIC_DELETE) && (flags & REPLACE_MODE))
	{
		// if both have just been enabled, arbitrarily disable one of them
		flags &= ~SPECIFIC_DELETE;
	}
	gameModel->GetSimulation()->replaceModeFlags = flags;
}

void GameController::OpenSearch(String searchText)
{
	if(!search)
		search = new SearchController([this] {
			if (search->GetLoadedSave())
			{
				try
				{
					HistorySnapshot();
					gameModel->SetSave(search->GetLoadedSave(), gameView->ShiftBehaviour());
					search->ReleaseLoadedSave();
				}
				catch(GameModelException & ex)
				{
					new ErrorMessage("Cannot open save", ByteString(ex.what()).FromUtf8());
				}
			}
		});
	if (searchText.length())
		search->DoSearch2(searchText);
	ui::Engine::Ref().ShowWindow(search->GetView());
}

void GameController::OpenLocalSaveWindow(bool asCurrent)
{
	Simulation * sim = gameModel->GetSimulation();
	GameSave * gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour());
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
			new LocalSaveActivity(tempSave, [this](SaveFile *file) {
				gameModel->SetSaveFile(file, gameView->ShiftBehaviour());
			});
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

			gameModel->SetSaveFile(&tempSave, gameView->ShiftBehaviour());
			Platform::MakeDirectory(LOCAL_SAVE_DIR);
			auto [ fromNewerVersion, saveData ] = gameSave->Serialise();
			(void)fromNewerVersion;
			if (saveData.size() == 0)
				new ErrorMessage("Error", "Unable to serialize game data.");
			else if (!Platform::WriteFile(saveData, gameModel->GetSaveFile()->GetName()))
				new ErrorMessage("Error", "Unable to write save file.");
			else
				gameModel->SetInfoTip("Saved Successfully");
		}
	}
}

void GameController::LoadSaveFile(SaveFile * file)
{
	gameModel->SetSaveFile(file, gameView->ShiftBehaviour());
}


void GameController::LoadSave(SaveInfo * save)
{
	gameModel->SetSave(save, gameView->ShiftBehaviour());
}

void GameController::OpenSaveDone()
{
	if (activePreview->GetDoOpen() && activePreview->GetSaveInfo())
	{
		try
		{
			HistorySnapshot();
			LoadSave(activePreview->GetSaveInfo());
		}
		catch(GameModelException & ex)
		{
			new ErrorMessage("Cannot open save", ByteString(ex.what()).FromUtf8());
		}
	}
}

void GameController::OpenSavePreview(int saveID, int saveDate, bool instant)
{
	activePreview = new PreviewController(saveID, saveDate, instant, [this] { OpenSaveDone(); });
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}

void GameController::OpenSavePreview()
{
	if(gameModel->GetSave())
	{
		activePreview = new PreviewController(gameModel->GetSave()->GetID(), 0, false, [this] { OpenSaveDone(); });
		ui::Engine::Ref().ShowWindow(activePreview->GetView());
	}
}

void GameController::OpenLocalBrowse()
{
	new FileBrowserActivity(LOCAL_SAVE_DIR PATH_SEP, [this](std::unique_ptr<SaveFile> file) {
		HistorySnapshot();
		LoadSaveFile(file.get());
	});
}

void GameController::OpenLogin()
{
	loginWindow = new LoginController();
	ui::Engine::Ref().ShowWindow(loginWindow->GetView());
}

void GameController::OpenProfile()
{
	if(Client::Ref().GetAuthUser().UserID)
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
	std::vector<Tool*> toolList;
	std::vector<Menu*> menuList = gameModel->GetMenuList();
	for (auto i = 0U; i < menuList.size(); ++i)
	{
		if (i == SC_FAVORITES)
		{
			continue;
		}
		auto *mm = menuList[i];
		if(!mm)
			continue;
		std::vector<Tool*> menuToolList = mm->GetToolList();
		if(!menuToolList.size())
			continue;
		toolList.insert(toolList.end(), menuToolList.begin(), menuToolList.end());
	}
	std::vector<Tool*> hiddenTools = gameModel->GetUnlistedTools();
	toolList.insert(toolList.end(), hiddenTools.begin(), hiddenTools.end());
	new ElementSearchActivity(this, toolList);
}

void GameController::OpenColourPicker()
{
	new ColourPickerActivity(gameModel->GetColourSelectorColour(), [this](ui::Colour colour) {
		SetColour(colour);
	});
}

void GameController::OpenTags()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetID())
	{
		delete tagsWindow;
		tagsWindow = new TagsController([this] { gameView->NotifySaveChanged(gameModel); }, gameModel->GetSave());
		ui::Engine::Ref().ShowWindow(tagsWindow->GetView());
	}
	else
	{
		new ErrorMessage("Error", "No save open");
	}
}

void GameController::OpenStamps()
{
	localBrowser = new LocalBrowserController([this] {
		SaveFile *file = localBrowser->GetSave();
		if (file)
		{
			if (file->GetError().length())
				new ErrorMessage("Error loading stamp", file->GetError());
			else if (localBrowser->GetMoveToFront())
				Client::Ref().MoveStampToFront(file->GetDisplayName().ToUtf8());
			LoadStamp(file->GetGameSave());
		}
	});
	ui::Engine::Ref().ShowWindow(localBrowser->GetView());
}

void GameController::OpenOptions()
{
	options = new OptionsController(gameModel, [this] {
		gameModel->UpdateQuickOptions();
		Client::Ref().WritePrefs(); // * I don't think there's a reason for this but I'm too lazy to check. -- LBPHacker
	});
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
	if(gameModel->GetUser().UserID)
	{
		Simulation * sim = gameModel->GetSimulation();
		GameSave * gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour());
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
				new ServerSaveActivity(tempSave, [this](SaveInfo &save) {
					save.SetVote(1);
					save.SetVotesUp(1);
					LoadSave(&save);
				});
			}
			else
			{
				SaveInfo tempSave(0, 0, 0, 0, 0, gameModel->GetUser().Username, "");
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, [this](SaveInfo &save) {
					save.SetVote(1);
					save.SetVotesUp(1);
					LoadSave(&save);
				});
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
	if(gameModel->GetSave() && gameModel->GetUser().UserID && gameModel->GetUser().Username == gameModel->GetSave()->GetUserName())
	{
		Simulation * sim = gameModel->GetSimulation();
		GameSave * gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour());
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
				new ServerSaveActivity(tempSave, true, [this](SaveInfo &save) { LoadSave(&save); });
			}
			else
			{
				SaveInfo tempSave(0, 0, 0, 0, 0, gameModel->GetUser().Username, "");
				tempSave.SetGameSave(gameSave);
				new ServerSaveActivity(tempSave, true, [this](SaveInfo &save) { LoadSave(&save); });
			}
		}
	}
	else if(gameModel->GetUser().UserID)
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
	if(gameModel->GetSave() && gameModel->GetUser().UserID && gameModel->GetSave()->GetID() && gameModel->GetSave()->GetVote()==0)
	{
		try
		{
			gameModel->SetVote(direction);
		}
		catch(GameModelException & ex)
		{
			new ErrorMessage("Error while voting", ByteString(ex.what()).FromUtf8());
		}
	}
}

void GameController::ChangeBrush()
{
	auto prev_size = gameModel->GetBrush()->GetRadius();
	gameModel->SetBrushID(gameModel->GetBrushID()+1);
	gameModel->GetBrush()->SetRadius(prev_size);
}

void GameController::ClearSim()
{
	HistorySnapshot();
	gameModel->SetSave(NULL, false);
	gameModel->ClearSimulation();
}

String GameController::ElementResolve(int type, int ctype)
{
	// "NONE" should never be displayed in the HUD
	if (!type)
		return "";
	if (gameModel && gameModel->GetSimulation())
	{
		return gameModel->GetSimulation()->ElementResolve(type, ctype);
	}
	return "";
}

String GameController::BasicParticleInfo(Particle const &sample_part)
{
	if (gameModel && gameModel->GetSimulation())
	{
		return gameModel->GetSimulation()->BasicParticleInfo(sample_part);
	}
	return "";
}

void GameController::ReloadSim()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSave(gameModel->GetSave(), gameView->ShiftBehaviour());
	}
	else if(gameModel->GetSaveFile() && gameModel->GetSaveFile()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSaveFile(gameModel->GetSaveFile(), gameView->ShiftBehaviour());
	}
}

bool GameController::IsValidElement(int type)
{
	if (gameModel && gameModel->GetSimulation())
	{
		return (type && gameModel->GetSimulation()->IsElement(type));
	}
	else
		return false;
}

String GameController::WallName(int type)
{
	if(gameModel && gameModel->GetSimulation() && type >= 0 && type < UI_WALLCOUNT)
		return gameModel->GetSimulation()->wtypes[type].name;
	else
		return String();
}

ByteString GameController::TakeScreenshot(int captureUI, int fileType)
{
	return gameView->TakeScreenshot(captureUI, fileType);
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

void GameController::NotifyNewNotification(Client * sender, std::pair<String, ByteString> notification)
{
	class LinkNotification : public Notification
	{
		ByteString link;
	public:
		LinkNotification(ByteString link_, String message) : Notification(message), link(link_) {}
		virtual ~LinkNotification() {}

		void Action() override
		{
			Platform::OpenURI(link);
		}
	};
	gameModel->AddNotification(new LinkNotification(notification.second, notification.first));
}

void GameController::NotifyUpdateAvailable(Client * sender)
{
	class UpdateNotification : public Notification
	{
		GameController * c;
	public:
		UpdateNotification(GameController * c, String message) : Notification(message), c(c) {}
		virtual ~UpdateNotification() {}

		void Action() override
		{
			UpdateInfo info = Client::Ref().GetUpdateInfo();
			StringBuilder updateMessage;
#ifndef MACOSX
			updateMessage << "Are you sure you want to run the updater? Please save any changes before updating.\n\nCurrent version:\n ";
#else
			updateMessage << "Click \"Continue\" to download the latest version from our website.\n\nCurrent version:\n ";
#endif

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
				updateMessage << info.Major << "." << info.Minor << " Beta, Build " << info.Build;
			else if (info.Type == UpdateInfo::Snapshot)
#if MOD_ID > 0
				updateMessage << "Mod version " << info.Time;
#else
				updateMessage << "Snapshot " << info.Time;
#endif
			else if(info.Type == UpdateInfo::Stable)
				updateMessage << info.Major << "." << info.Minor << " Stable, Build " << info.Build;

			if (info.Changelog.length())
				updateMessage << "\n\nChangelog:\n" << info.Changelog;

			new ConfirmPrompt("Run Updater", updateMessage.Build(), { [this] { c->RunUpdater(); } });
		}
	};

	switch(sender->GetUpdateInfo().Type)
	{
		case UpdateInfo::Snapshot:
#if MOD_ID > 0
			gameModel->AddNotification(new UpdateNotification(this, "A new mod update is available - click here to update"));
#else
			gameModel->AddNotification(new UpdateNotification(this, "A new snapshot is available - click here to update"));
#endif
			break;
		case UpdateInfo::Stable:
			gameModel->AddNotification(new UpdateNotification(this, "A new version is available - click here to update"));
			break;
		case UpdateInfo::Beta:
			gameModel->AddNotification(new UpdateNotification(this, "A new beta is available - click here to update"));
			break;
	}
}

void GameController::RemoveNotification(Notification * notification)
{
	gameModel->RemoveNotification(notification);
}

void GameController::RunUpdater()
{
#ifndef MACOSX
	Exit();
	new UpdateActivity();
#else

#ifdef UPDATESERVER
	ByteString file = ByteString::Build(SCHEME, UPDATESERVER, Client::Ref().GetUpdateInfo().File);
#else
	ByteString file = ByteString::Build(SCHEME, SERVER, Client::Ref().GetUpdateInfo().File);
#endif

	Platform::OpenURI(file);
#endif // MACOSX
}

bool GameController::GetMouseClickRequired()
{
	return gameModel->GetMouseClickRequired();
}

void GameController::RemoveCustomGOLType(const ByteString &identifier)
{
	gameModel->RemoveCustomGOLType(identifier);
}
